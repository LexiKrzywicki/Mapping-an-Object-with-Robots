
#include <Romi32U4.h>
#include "Wall_Follower.h"

IR_sensor irSensor;
SpeedController speedController;
//Romi32U4Motors motorS;

float thetaThresh = .08;//.06;//0.10;

void Wall_Follower::Init()
{
}

void Wall_Follower::wallFollow(float dist, float Kp, float Kd, float target_velocity, unsigned long lastTime)
{
     /*Serial.print(irSensor.ReadData());
     Serial.print("\t");
     Serial.print(speedController.ReadLeftVelocity());
     Serial.print("\t");
     Serial.println(speedController.ReadRightVelocity());*/
    // error in distance from the wall
    float e_dist = dist - irSensor.ReadData();

    // the previous value of e_dist
    static float prev_e_dist = 0.0;
    
    unsigned long currTime = millis();
    float deltaTime = (float)(currTime - lastTime);
    
    //change in e_dist
    float delta_e_dist = (e_dist - prev_e_dist) / deltaTime;

    // difference between left and right velocities
    float deltaVelocity = Kp * e_dist - Kd * delta_e_dist;


    speedController.Process(target_velocity + (0.5 * deltaVelocity), target_velocity - (0.5 * deltaVelocity));
    
    
}

void Wall_Follower::wallFollowWithout(float dist, float Kp, float Kd, float target_velocity, unsigned long lastTime)
{
    // error in distance from the wall
    float e_dist = dist - irSensor.ReadData();
   

    // the previous value of e_dist
    static float prev_e_dist = 0.0;
    
    unsigned long currTime = millis();
    float deltaTime = (float)(currTime - lastTime);
    
    //change in e_dist
    float delta_e_dist = (e_dist - prev_e_dist) / deltaTime;

    // difference between left and right velocities
    float deltaVelocity = Kp * e_dist + Kd * delta_e_dist;

    //motorS.setEfforts(target_velocity + .5*deltaVelocity, target_velocity - .5*deltaVelocity);
    
    
}

void Wall_Follower::stop()
{
    speedController.Stop();
}

float Wall_Follower::readLeftVelocity()
{
    return speedController.readLeftVelocity();
}


float Wall_Follower::readRightVelocity()
{
    return speedController.readRightVelocity();
}

bool Wall_Follower::turn(float currTheta, float rads)
{ 
     Serial.print(currTheta);
     Serial.print("\t");
     Serial.println(rads);
    speedController.Process(30.0, -30.0);

    if(abs(rads - currTheta) < thetaThresh)
    { 
        speedController.Stop();
        return true;
    }
    return false;
} 
//
bool Wall_Follower::turnP(float currTheta, float rads)
{ 
    float kP = 50.0;

    //  Serial.print(currTheta);
    //  Serial.print("\t");
    //  Serial.println(rads);

     float thetaErr = currTheta - rads;

    if (thetaErr > PI)
    {
        thetaErr -= 2 * PI;
        thetaErr *= -1;
    }

    //Serial.println((thetaErr * kP));

    speedController.Process(abs(thetaErr * kP), -abs(thetaErr * kP));

    if(abs(rads - currTheta) < thetaThresh)
    { 
        speedController.Stop();
        return true;
    }
    return false;
} 


int count = 0; 
bool Wall_Follower::pTurn(float currTheta, float rads)
{ 
    // Serial.print(currTheta);
    // Serial.print("\t");
    // Serial.println(rads);

    float thetaError = currTheta - rads;

    if (thetaError > PI)
    {
        thetaError -= PI;
        thetaError *= -1;
    }
    float kP = 40;
    



    speedController.Process(-thetaError * kP, thetaError * kP);

    if(abs(rads - currTheta) < .08)
    { 
        speedController.Stop();
        count++;
    }

    if (count > 100)
    {
        count = 0;
        return true;
    }
    return false;
} 


void Wall_Follower::Process(float speedLeft, float speedRight)
{
    speedController.Process(speedLeft, speedRight);
}

float KP_THETA = 35.0;

void Wall_Follower::MoveByTheta (float target_Theta, float currTheta)
{

    //calculate correction to angle driving at
    float thetaError = target_Theta - currTheta; // when positive need to turn CCW

    if (abs(thetaError) > 180)
    {
      thetaError = (2 * PI - thetaError);
    }

    float errorCorrection = KP_THETA * thetaError;

    //drive
    speedController.Process(30.0 - (.5 * errorCorrection), 30.0 + (.5 * errorCorrection));

    //Serial.println(targetTheta);

  
}