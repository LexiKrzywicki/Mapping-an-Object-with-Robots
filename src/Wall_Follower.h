#ifndef WALL_FOLLOWER
#define WALL_FOLLOWER

#include <Romi32U4.h>
#include <IR_sensor.h>
#include "Speed_controller.h"

class Wall_Follower
{
private:
    float Kp = 1.0;
    float Kd = 0;
    IR_sensor irSensor;
    SpeedController speedController;
    

public:
    void Init();
    void wallFollow(float dist, float Kp, float Kd, float target_velocity, unsigned long lastTime);
    void wallFollowWithout(float dist, float Kp, float Kd, float target_velocity, unsigned long lastTime);
    void stop();
    float readLeftVelocity();
    float readRightVelocity();
    bool turn(float currTheta, float radians);
    bool pTurn(float currTheta, float radians);
    void Process(float speedLeft, float speedRight);
    bool turnP(float currTheta, float rads);
    void MoveByTheta (float target_Theta, float currTheta);

};

#endif