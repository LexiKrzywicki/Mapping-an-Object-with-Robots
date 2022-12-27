#include <Arduino.h>
#include <Wall_Follower.h>
#include <Position_estimation.h>
#include "apriltagdatum.h"
#include "openmv.h"
#include <PCint.h>
#include <Rangefinder.h>



enum RobotState {IDLE, NORTH, EAST, SOUTH, WEST, TRANSMIT};
RobotState robotState = IDLE;

Wall_Follower wallFollower;
Romi32U4ButtonA buttonA;
Romi32U4ButtonB buttonB;
Romi32U4ButtonC buttonC;
Position position;


AprilTagDatum tag;
OpenMV camera;

float prevX = 0.0;
float prevY = 0.0;
bool turning = false;

Rangefinder rangefinder(3, 2); //echo 3 trig 2

//camera variables
float tagCXPosToTurn = 60; //TODO @ANE tune
float tagHToTurn = 76;//73;
float pixelThresh = 5;
int NORTH_ID = 2;
int EAST_ID = 3;
int SOUTH_ID = 0;
int WEST_ID = 1;
const int numReadings = 5;
const int medianNum = numReadings/2;

float readingArray[numReadings];
float sortedArray[numReadings];


//wall follow variables
unsigned long lastTime = 0.0;
float dist = 24;//27;//31;   // 43 actually
float wallKp = 6.0; //5.0; //6.0;//21.0; // 9.7; 
float wallKd = 1.0; //5.0;  //1.0//.1351;
float target_velocity = 35.0;

float turnStartTheta = 0.0;

//matrix function(s)
/*
smallest wall 9cm
largest wall 17 cm
field 125cm by 125 cm
5 cm between samples
*/

unsigned long startTime = 0.0;
unsigned long prevTimed = 0.0;
unsigned long z = 0.0;




const int FIELD_LENGTH = 19;
const int FIELD_WIDTH = 19;
float pixelSize = 125.0/19.0;
const float mmBetweenSamples = 30;//pixelSize/2.0 *10;

//true in this matrix means there is something in that position
bool matrix[FIELD_LENGTH][FIELD_WIDTH]; 

float sort(void)
{
  int i, j;
  float maxVal = __FLT_MAX__;
  float minVal = maxVal;
  for(i = 0; i < numReadings; i++)
  {
    for(j = 0; j < numReadings; j++)
    {
      if(readingArray[j] < minVal)
      {
        sortedArray[i] = readingArray[j];
        minVal = readingArray[j];
        readingArray[j] = maxVal;
      }
    }
  }
  return sortedArray[medianNum];
}

//copied from Lewin
void sendMessage(const String& topic, const String& message)
{
    Serial1.println(topic + String(':') + message);
}

/*
xPixel is the x position of the pixel
yPixel is the y position of the pixel
side a string representing the side of the arena the romi is on: 1 = N, 2 = E, 3 = S, 4 = W
*/
bool sendESP(int xPixel, int yPixel, String side)
{
  String topic = "team13/(";
  topic += xPixel;
  topic += ",";
  topic += yPixel;
  topic += ")";
  
  sendMessage(topic, side);
}

//fills matrix with false in all positions
void matrixInit()
{
  for(int i = 0; i < FIELD_LENGTH; i++)
  {
    for(int j = 0; j < FIELD_WIDTH; j++)
    {
      matrix[i][j] = false;
    }
  }
}

//marks the point of something if it is in the area (ignores area boundries)
void handleNorthReading(float reading, float currX, float currY, float currTheta)
{  
  float mCurrY = -currY / 10.0; 
  float realCurrX = currX / 10.0;
  float targetTheta = 0.0;
  float ThetaE; 
  int pixelX = 0;
  int pixelY = 0;

  if(currTheta > PI){
    ThetaE = targetTheta + (2*PI - currTheta);
    pixelX = (realCurrX - reading*sin(ThetaE))/pixelSize;
    pixelY = (mCurrY + reading*cos(ThetaE))/pixelSize; //careful of weird coordinate system

  }else{
    ThetaE = currTheta - targetTheta;
    pixelX = (realCurrX + reading*sin(ThetaE))/pixelSize;
    pixelY = (mCurrY + reading*cos(ThetaE))/pixelSize; //careful of weird coordinate system
  }


  // Serial.print(pixelX);
  // Serial.print("\t");
  // Serial.println(pixelY);
  if(pixelX < FIELD_LENGTH && pixelX > 0 && pixelY < FIELD_WIDTH && pixelY >0){
    sendESP(pixelX, pixelY, "1");
  }

}

//marks the point of something if it is in the area (ignores area boundries)
void handleSouthReading(float reading, float currX, float currY, float currTheta)
{  
  float mCurrY = -currY / 10.0; 
  float realCurrX = currX / 10.0;
  float targetTheta = PI;
  float ThetaE; 
  int pixelX = 0;
  int pixelY = 0;

  
  ThetaE = currTheta - targetTheta;

  if(ThetaE < 0)
  {
    pixelX = (realCurrX - reading*sin(ThetaE))/pixelSize;
    pixelY = (mCurrY - reading*cos(ThetaE))/pixelSize; //careful of weird coordinate system
  }else {
    pixelX = (realCurrX - reading*sin(ThetaE))/pixelSize;
    pixelY = (mCurrY - reading*cos(ThetaE))/pixelSize; //careful of weird coordinate system
  }
  

  // Serial.print(pixelX);
  // Serial.print("\t");
  // Serial.println(pixelY);
  if(pixelX < FIELD_LENGTH && pixelX > 0 && pixelY < FIELD_WIDTH && pixelY >0){
    sendESP(pixelX, pixelY, "3");
  }
  //}


}

//marks the point of something if it is in the area (ignores area boundries)
void handleWestReading(float reading, float currX, float currY, float currTheta)
{  
  float mCurrY = -currY / 10.0; 
  float realCurrX = currX / 10.0;
  float targetTheta = PI/2;
  float ThetaE; 
  int pixelX = 0;
  int pixelY = 0;

  
  ThetaE = currTheta - targetTheta;

  if(ThetaE < 0)
  {
    pixelX = (realCurrX - reading*sin(ThetaE))/pixelSize;
    pixelY = (mCurrY + reading*cos(ThetaE))/pixelSize; //careful of weird coordinate system
  }
  else 
  {
    pixelX = (realCurrX + reading*sin(ThetaE))/pixelSize;
    pixelY = (mCurrY - reading*cos(ThetaE))/pixelSize; //careful of weird coordinate system
  }


  // Serial.print(pixelX);
  // Serial.print("\t");
  // Serial.println(pixelY);
  if(pixelX < FIELD_LENGTH && pixelX > 0 && pixelY < FIELD_WIDTH && pixelY >0){
    sendESP(pixelX, pixelY, "4");
  }
  //}

}


//marks the point of something if it is in the area (ignores area boundries)
void handleEastReading(float reading, float currX, float currY, float currTheta)
{  
  float mCurrY = -currY / 10.0; 
  float realCurrX = currX / 10.0;
  float targetTheta = 3*PI/2;
  float ThetaE; 
  int pixelX = 0;
  int pixelY = 0;

  
  ThetaE = currTheta - targetTheta;

  if(ThetaE < 0)
  {
    pixelX = (realCurrX - reading*sin(ThetaE))/pixelSize;
    pixelY = (mCurrY + reading*cos(ThetaE))/pixelSize; //careful of weird coordinate system
  }else {
    pixelX = (realCurrX - reading*sin(ThetaE))/pixelSize;
    pixelY = (mCurrY + reading*cos(ThetaE))/pixelSize; //careful of weird coordinate system
  }
  

  // Serial.print(pixelX);
  // Serial.print("\t");
  // Serial.println(pixelY);
  if(pixelX < FIELD_LENGTH && pixelX > 0 && pixelY < FIELD_WIDTH && pixelY >0){
    sendESP(pixelX, pixelY, "2");
  }
  //}

}

//fills in gaps in matrix that are enclosed by something in the way
void finishMatrix()
{
  for(int i = 0; i < 24; i++)
  {
    /*
    firstTrueFlag is a boolean flag that tells if the row has reached a point in the matrix where the
    pixel[r][c] is true
    */ 
    bool firstTrueFlag = false;

    /*
    firstTrue is the first point in a row where there is something there
    */
    int firstTrue = 0;

    /*
    lastTrue is the last point in a row where there is something there
    */
    int lastTrue = 0;

    for(int j = 0; j < 24; j++)
    {
      if(matrix[i][j])
      {
        if(!firstTrueFlag)
        {
          firstTrue = j;
        }
        lastTrue = j;
      }
    }

    if(firstTrueFlag)
    {
      for(int k = 0; k < 24; k++)
      {
        if(k > firstTrue && k < lastTrue)
        {
          matrix[i][k] = true;
        }
      }
    }

  }
}

void takeSample()
{
  //placeholder
}

bool tagInPosition (int tagID)
{
  camera.checkUART(tag);
/*
  Serial.print(tag.id);
  Serial.println("\t");
  Serial.println(tag.cx);*/

  if (tag.id == tagID && abs(tag.h - tagHToTurn) < pixelThresh)//(tag.cx - tagCXPosToTurn) < pixelThresh)
  {
    return true;
  }
  return false;
}

//Modify wall follower to take in a speed controller 

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial1.begin(115200);
  rangefinder.init();
  //matrixInit();
}

bool nextDrive = false;

void loop() {
//Serial1.send("cat", "cats");
  /*sendESP(3,4,"2");
  delay(500);*/
  //handleSouthReading( 25, 0, 0, (PI) - PI/6);
  position.UpdatePose(wallFollower.readLeftVelocity(), wallFollower.readRightVelocity());
  //position.PrintPose();

  /*
  camera.checkUART(tag);
  Serial.print((float)tag.id); 
  Serial.print("\t");
  Serial.print(tag.h);
  Serial.print("\t");
  Serial.println(tag.cx);
 */
/*
  if (buttonB.getSingleDebouncedRelease() == true)
      {
       // position.reset();
       Serial.println("start turn");
       turning = true;
      }
/*
  if (turning == true && wallFollower.turnP(position.ReadPose().THETA, 3 * PI/2) == true)
  {
    //delay(50);
    position.UpdatePose(wallFollower.readLeftVelocity(), wallFollower.readRightVelocity());
    position.PrintPose();
    wallFollower.stop();
    // delay(5000);
    // position.UpdatePose(wallFollower.readLeftVelocity(), wallFollower.readRightVelocity());
    // wallFollower.Process(30.0,30.0);
    // delay(5000);
    // wallFollower.stop();
    // position.UpdatePose(wallFollower.readLeftVelocity(), wallFollower.readRightVelocity());
    // position.PrintPose();
    turning = false;
    
  }

    if (buttonC.getSingleDebouncedRelease() == true)
      {
       // position.reset();
       Serial.println("start turn");
       nextDrive = !nextDrive;
      }

   if (nextDrive)
  {
    wallFollower.Process(30.0, 30.0);
  }
*/
  switch (robotState)
  {
    case IDLE:
     //Serial.println("IDLE");
      if (buttonA.getSingleDebouncedRelease() == true)
      {
        Serial.println("GOING NORTH");
        robotState = NORTH;
      }
       wallFollower.stop();//.wallFollow(dist, wallKp, wallKd, target_velocity, lastTime);//
       lastTime = millis();
       
    break;

    case NORTH:
    //Serial.println("IN NORTH");
    z = millis();
        if (z-prevTimed >= 250/*position.ReadPose().X - prevX > mmBetweenSamples || position.ReadPose().Y - prevY  > mmBetweenSamples*/)
        {
          prevX = position.ReadPose().X;
          prevY = position.ReadPose().Y;
          int k;
          for(k = 0; k < numReadings; k++)
          {
            readingArray[k] = rangefinder.getDistance();
          }
          handleNorthReading(sort(), position.ReadPose().X, position.ReadPose().Y, position.ReadPose().THETA);
          //Serial.println("SAMPLING");
          prevTimed = millis();
          //Serial.println(lastTime);
        }

    camera.checkUART(tag);
     //Serial.println((float)tag.id); 
  ///Serial.println("\t");
  //Serial.println(tag.cx);
     if (turning == false && tagInPosition(NORTH_ID) == true)
     {
       
      //  wallFollower.Process(target_velocity, target_velocity);
      //  delay(500);
       turnStartTheta = position.ReadPose().THETA;
       turning = true;
      startTime = millis();
       Serial.println("WE SAW THE NORTH TAG!!!");
     }
     
     
     if (turning == true && wallFollower.turnP(position.ReadPose().THETA, 3 * PI/2) == true)//turnStartTheta + (PI /2)) == true)//
     {

      //  Serial.println("GOING EAST");
      //  wallFollower.Process(-30.0, 30.0);
      //      Serial.println("FINISHED TURN");
      // robotState = EAST;
      // turning = false;
      //  }
       Serial.println("FINISHED TURN");
       prevY = position.ReadPose().Y;
       Serial.println("GOING EAST");
      //  wallFollower.Process(-30.0, 30.0);
      //  delay(50);
      robotState = EAST;
      turning = false;
     }
     else if (turning == false)
     {
       //Serial.println("TRYING TO WALL FOLLOW");
       wallFollower.wallFollow(dist, wallKp, wallKd, target_velocity, lastTime);
       //wallFollower.MoveByTheta(0, position.ReadPose().THETA);
       //wallFollower.Process(target_velocity, target_velocity);
       lastTime = millis();
     }
    break;

    case EAST:
    //Serial.println("IN EAST");
      /*if (position.ReadPose().X - prevX  > mmBetweenSamples)
        {
          prevX = position.ReadPose().X;
          takeSample();
          Serial.println("SAMPLING");
        }*/
        if (turning == false && millis()-prevTimed >= 250/*abs(position.ReadPose().X - prevX) > mmBetweenSamples || abs(position.ReadPose().Y - prevY)  > mmBetweenSamples*/)
        {
          prevX = position.ReadPose().X;
          prevY = position.ReadPose().Y;
          int k;
          for(int k = 0; k < numReadings; k++)
          {
            readingArray[k] = rangefinder.getDistance();
          }
          float sorted = sort();
          handleEastReading(sorted, position.ReadPose().X, position.ReadPose().Y, position.ReadPose().THETA);
          Serial.println(sorted);
          prevTimed = millis();
        }

    camera.checkUART(tag);
     //Serial.println((float)tag.id); 
  ///Serial.println("\t");
  //Serial.println(tag.cx);
     if (turning == false && tagInPosition(EAST_ID) == true)
     {
      //  wallFollower.Process(target_velocity, target_velocity);
      //  delay(800);
       turnStartTheta = position.ReadPose().THETA;
       turning = true;

       Serial.println("WE SAW THE EAST TAG!!!");
     }
     
     
     if (turning == true && wallFollower.turnP(position.ReadPose().THETA, PI) == true)//turnStartTheta - (PI /2)) == true)//
     {
       Serial.println("FINISHED TURN");
       prevY = position.ReadPose().Y;
       Serial.println("GOING SOUTH");
      //  wallFollower.Process(-30.0, 30.0);
      //  delay(50);
      robotState = SOUTH;
      turning = false;
     }
     else if (turning == false)
     {
       //Serial.println("TRYING TO WALL FOLLOW");
       wallFollower.wallFollow(dist, wallKp, wallKd, target_velocity, lastTime);
       lastTime = millis();
     }
    break;
    

    case SOUTH:
    //Serial.println("IN EAST");
      /*if (position.ReadPose().X - prevX  > mmBetweenSamples)
        {
          prevX = position.ReadPose().X;
          takeSample();
          Serial.println("SAMPLING");
        }*/

        if (millis()-prevTimed >= 250/*position.ReadPose().X - prevX > mmBetweenSamples || position.ReadPose().Y - prevY  > mmBetweenSamples*/)
        {
          prevX = position.ReadPose().X;
          prevY = position.ReadPose().Y;
          int k;
          for(k = 0; k < numReadings; k++)
          {
            readingArray[k] = rangefinder.getDistance();
          }
          handleSouthReading(sort(), position.ReadPose().X, position.ReadPose().Y, position.ReadPose().THETA);
          //Serial.println("SAMPLING");
          prevTimed = millis();
        }

    //Serial.println("IN SOUTH");
     //Serial.println((float)tag.id); 
  ///Serial.println("\t");
  //Serial.println(tag.cx); 
     if (turning == false && tagInPosition(SOUTH_ID) == true)
     {
      //  wallFollower.Process(target_velocity, target_velocity);
      //  delay(800);
       turnStartTheta = position.ReadPose().THETA;
       turning = true;

       Serial.println("WE SAW THE SOUTH TAG!!!");
     }
     
    camera.checkUART(tag);
     
     if (turning == true && wallFollower.turnP(position.ReadPose().THETA, PI/2) == true)//turnStartTheta - (PI /2)) == true)//
     {
       Serial.println("FINISHED TURN");
       prevY = position.ReadPose().Y;
       
       Serial.println("GOING WEST");
      //  wallFollower.Process(-30.0, 30.0);
      //  delay(50);
      robotState = WEST;
      turning = false;
     }
     else if (turning == false)
     {
       //Serial.println("TRYING TO WALL FOLLOW");
       wallFollower.wallFollow(dist, wallKp, wallKd, target_velocity, lastTime);
       lastTime = millis();
     }
    break;

    case WEST:
    //Serial.println("IN WEST");
      /*if (position.ReadPose().X - prevX  > mmBetweenSamples)
        {
          prevX = position.ReadPose().X;
          takeSample();
          Serial.println("SAMPLING");
        }*/

        if (millis()-prevTimed >= 250/*position.ReadPose().X - prevX > mmBetweenSamples || position.ReadPose().Y - prevY  > mmBetweenSamples*/)
        {
          prevX = position.ReadPose().X;
          prevY = position.ReadPose().Y;
          int k;
          for(k = 0; k < numReadings; k++)
          {
            readingArray[k] = rangefinder.getDistance();
          }
          handleWestReading(sort(), position.ReadPose().X, position.ReadPose().Y, position.ReadPose().THETA);
          //Serial.println("SAMPLING");
          prevTimed = millis();
        }

    camera.checkUART(tag);
     //Serial.println((float)tag.id); 
  ///Serial.println("\t");
  //Serial.println(tag.cx);
     if (turning == false && tagInPosition(WEST_ID) == true)
     {
      //  wallFollower.Process(target_velocity, target_velocity);
      //  delay(800);
       turnStartTheta = position.ReadPose().THETA; 
       turning = true;

       Serial.println("WE SAW THE WEST TAG!!!");
     }
     
     
     if (turning == true && wallFollower.turnP(position.ReadPose().THETA, 0) == true)//turnStartTheta - (PI /2)) == true)//
     {
       Serial.println("FINISHED TURN");
       prevY = position.ReadPose().Y;
      robotState = IDLE;//TRANSMIT;
      turning = false;
     }
     else if (turning == false)
     {
       //Serial.println("TRYING TO WALL FOLLOW");
       wallFollower.wallFollow(dist, wallKp, wallKd, target_velocity, lastTime);
       lastTime = millis();
     }
    break;

    case TRANSMIT:
      wallFollower.stop();
      Serial.println("TRANSMITTING");
      //finishMatrix();


    break;


  }
  
}
        