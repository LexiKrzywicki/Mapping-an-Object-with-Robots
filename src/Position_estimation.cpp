#include  "Position_estimation.h"

float x = 0;
float y = 0;
float theta = 0;


unsigned long currTime = 0;
unsigned long prevTime = 0;
float THRESHOLD = 5.0;
float WHEEL_BASE = 150; //TODO @ANE

void Position::Init(void)
{
    //set up times
    prevTime = millis();
    currTime = millis();

    x = 0;
    y = 0;
    theta = 0;

}

void Position::Stop(void)
{
    //
}

Position::pose_data Position::ReadPose(void)
{
    return {x,y,theta};
}

void Position::PrintPose(void)
{
    Serial.print(x);
    Serial.print('\t');
    Serial.print(y);
    Serial.print('\t');
    Serial.print(theta);
    Serial.print('\t');
    Serial.println(theta/(PI) * 180);
}

void Position::reset()
{
    x = 0;
    y = 0;
    theta = 0;
}

void Position::UpdatePose(float target_speed_left, float target_speed_right)
{

    currTime = millis();

    unsigned long elapsedTime = currTime - prevTime;  //millis 

    //distance travelled
    double distLeft = (double)(elapsedTime / 1000.0) * (double)target_speed_left;
    double distRight = (double)(elapsedTime / 1000.0) * (double)target_speed_right; 
    double dist = (distLeft + distRight) / 2;

    //Serial.print(elapsedTime, 3);
   

    //driving straight
    if (abs(target_speed_left - target_speed_right) < THRESHOLD)
    {
        //theta stays the same 

        

        //    /|
        //   / |
        //  o--|      
        //x increases by dist * cos(theta)
        x += (float)(dist * cos(double(theta)));
        //Serial.println(x);

        //y increases by dist * sin(theta)
        y +=  (float)(dist * sin(double(theta)));


    }
    //pivot around center
    else if (abs(abs(target_speed_left) - abs(target_speed_right)) < THRESHOLD)
    {
        //x and y do not change
        x = x;
        y = y;

        //theta changes based on the wheel base and the distance travelled 

        float diameter = WHEEL_BASE;
        float radius = .5 * WHEEL_BASE;

        float leftRadians = (float)distLeft / radius;
        float rightRadians = (float)distRight / radius;

        //averages radian readouts from the two sides and accounts for direction while adding to theta 
        float radians = (abs(leftRadians) + abs(rightRadians)) / 2;
        if (target_speed_right > 0)
        {
        theta += radians;
        }
        else
        {
        theta -= radians;
        }
    }
    //curved motion 
    else 
    {
        //rL = rL
        //rR = rL + WHEEL_BASE

        //arcL = distLeft = angle/
        //arcR = distRight


        float angVel = (target_speed_right - target_speed_left) / WHEEL_BASE;
        float deltaTheta = (angVel * elapsedTime / 1000);

        //Serial.print("angVel : ");
        //Serial.println(angVel, 5);

        float radius = (WHEEL_BASE / 2.0) * ((target_speed_right + target_speed_left) / (target_speed_right - target_speed_left));
        float deltaX = -radius * (float)sin((double)theta) + radius * (float)sin((double) (theta + deltaTheta));
        float deltaY = radius * (float)cos((double)theta) - radius * (float)cos((double) (theta + deltaTheta));


        this->theta += deltaTheta;
        this->x += deltaX;
        this->y += deltaY;


    }

   while (theta > (2 * PI))
   {
       theta = theta - (2 * PI);
   }

   while (theta < 0)
   {
       theta = theta + (2 * PI);
   }

    //theta = theta % 2;
    prevTime = currTime;


}
