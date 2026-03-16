#include "PWM.h"

// ================= Constructor =================
PWM::PWM(int deadLeft, int deadRight)
{
    DEAD_L = deadLeft;
    DEAD_R = deadRight;

    gainLeft = 1.0f;
    gainRight = 1.0f;
}

// ================= Init =================
void PWM::begin()
{
    ledcSetup(CH_G1, 20000, PWM_RESOLUTION);
    ledcSetup(CH_G2, 20000, PWM_RESOLUTION);
    ledcSetup(CH_D1, 20000, PWM_RESOLUTION);
    ledcSetup(CH_D2, 20000, PWM_RESOLUTION);

    ledcAttachPin(G_IN1, CH_G1);
    ledcAttachPin(G_IN2, CH_G2);
    ledcAttachPin(D_IN1, CH_D1);
    ledcAttachPin(D_IN2, CH_D2);
}

// ================= Symmetric drive =================
void PWM::driveLeft(int alpha)
{
    alpha = constrain(alpha, -ALPHA_MAX, ALPHA_MAX);

    int in1 = PWM_CENTER - alpha;
    int in2 = PWM_CENTER + alpha;

    ledcWrite(CH_G1, in1);
    ledcWrite(CH_G2, in2);
}

void PWM::driveRight(int alpha)
{
    alpha = constrain(alpha, -ALPHA_MAX, ALPHA_MAX);

    int in1 = PWM_CENTER + alpha;
    int in2 = PWM_CENTER - alpha;

    ledcWrite(CH_D1, in1);
    ledcWrite(CH_D2, in2);
}

// ================= Deadzone =================
int PWM::deadzoneShift(int u, int D)
{
    if (u == 0)
        return 0;

    if (u > 0)
        u = u + D;
    else
        u = u - D;

    return constrain(u, -PWM_MAX, PWM_MAX);
}

// ================= Gain setters =================
void PWM::setGainLeft(float g)
{
    gainLeft = constrain(g, 0.5f, 1.5f);
}

void PWM::setGainRight(float g)
{
    gainRight = constrain(g, 0.5f, 1.5f);
}

float PWM::getGainLeft()
{
    return gainLeft;
}

float PWM::getGainRight()
{
    return gainRight;
}

// ================= Outside interface =================
void PWM::setSpeed(int speed)
{
    int speed_l = deadzoneShift(speed, DEAD_L);
    int speed_r = deadzoneShift(speed, DEAD_R);

    // apply left/right correction
    speed_l = (int)(speed_l * gainLeft);
    speed_r = (int)(speed_r * gainRight);

    speed_l = constrain(speed_l, -1000, 1000);
    speed_r = constrain(speed_r, -1000, 1000);

    int alpha_l = map(speed_l,
                      -1000, 1000,
                      -ALPHA_MAX, ALPHA_MAX);

    int alpha_r = map(speed_r,
                      -1000, 1000,
                      -ALPHA_MAX, ALPHA_MAX);

    driveLeft(alpha_l);
    driveRight(alpha_r);
}

void PWM::stop()
{
    driveLeft(0);
    driveRight(0);
}