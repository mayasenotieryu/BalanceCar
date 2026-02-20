#include "PWM.h"

// ================= 构造函数 =================
PWM::PWM(int deadLeft, int deadRight)
{
    DEAD_L = deadLeft;
    DEAD_R = deadRight;
}

// ================= 初始化 =================
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

// ================= symmetric 驱动 =================
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

// ================= 死区补偿 =================
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

// ================= 外部接口 =================
void PWM::setSpeed(int speed)
{
    int speed_l = constrain(deadzoneShift(speed, DEAD_L), -1000, 1000);
    int speed_r = constrain(deadzoneShift(speed, DEAD_R), -1000, 1000);

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