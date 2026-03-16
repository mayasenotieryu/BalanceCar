#ifndef PWM_H
#define PWM_H

#include <Arduino.h>

class PWM
{
public:
    PWM();

    void begin();
    void setSpeed(int speed);                 // same command to both motors
    void setSpeedLR(int speedL, int speedR); // independent left/right commands
    void stop();

    void setGainLeft(float g);
    void setGainRight(float g);
    float getGainLeft();
    float getGainRight();

private:
    float gainLeft;
    float gainRight;

    static const int PWM_RESOLUTION = 10;
    static const int PWM_MAX = 1023;
    static const int PWM_CENTER = PWM_MAX / 2;
    static const int ALPHA_MAX = PWM_CENTER - 1;

    static const int G_IN1 = 17;
    static const int G_IN2 = 18;
    static const int D_IN1 = 16;
    static const int D_IN2 = 4;

    static const int CH_G1 = 0;
    static const int CH_G2 = 1;
    static const int CH_D1 = 2;
    static const int CH_D2 = 3;

    void driveLeft(int alpha);
    void driveRight(int alpha);
};

#endif