#ifndef _SUPERCAR_H__
#define _SUPERCAR_H__

typedef struct carControl
{
    float MotorPWM = 0.0f;
} CarControl_t;

class SuperCar
{
  public:
    void startTask();
    void running();
};

extern SuperCar BalanceCar;

#endif
