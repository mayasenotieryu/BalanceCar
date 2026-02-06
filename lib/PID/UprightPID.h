#ifndef Upright_h
#define Upright_h

typedef struct upRightPID
{
    volatile float Kp;
    volatile float Ki;
    volatile float Kd;
    volatile float Limit = 60.0f;

    volatile float Ki_Out = 0.0f;
    volatile float Kp_Min = -Limit;
    volatile float Kp_Max = Limit;

    volatile float Ki_Min = -Limit;
    volatile float Ki_Max = Limit;

    volatile float Kd_Min = -Limit;
    volatile float Kd_Max = Limit;

    volatile float outMin = -Limit;
    volatile float outMax = Limit;

    volatile float PID_Out = 0.0f;

} UprightPID_t;

void UprightPID_Init(UprightPID_t *pidConf, float Kp, float Ki, float Kd, float limit);

float UprightPID(UprightPID_t *pidConf, float targetAngleX, float angleX, float GyroX);

#endif