#ifndef PWM_H
#define PWM_H

#include <Arduino.h>

class PWM
{
public:
    PWM(int deadLeft = 470, int deadRight = 430);

    void begin();
    void setSpeed(int speed);   // -1000 ~ 1000
    void stop();

    // gain tuning
    void setGainLeft(float g);
    void setGainRight(float g);

    float getGainLeft();
    float getGainRight();

private:
    // deadzone
    int DEAD_L;
    int DEAD_R;

    // left/right correction gains
    float gainLeft;
    float gainRight;

    // PWM parameters
    static const int PWM_RESOLUTION = 10;
    static const int PWM_MAX = 1023;
    static const int PWM_CENTER = PWM_MAX / 2;
    static const int ALPHA_MAX = PWM_CENTER - 1;

    // pins
    static const int G_IN1 = 17;
    static const int G_IN2 = 18;
    static const int D_IN1 = 16;
    static const int D_IN2 = 4;

    // PWM channels
    static const int CH_G1 = 0;
    static const int CH_G2 = 1;
    static const int CH_D1 = 2;
    static const int CH_D2 = 3;

    // internal functions
    int deadzoneShift(int u, int D);
    void driveLeft(int alpha);
    void driveRight(int alpha);
};

#endif