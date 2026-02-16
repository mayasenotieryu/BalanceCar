#ifndef IMU_H
#define IMU_H

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

class IMU {
public:
    IMU();

    bool begin();
    void startTask();
    float getAngle();

private:
    static void taskWrapper(void *param);
    void update();

    Adafruit_MPU6050 mpu;

    float tetagr;
    float tetaomg;
    float tetaF;

    float Te;
    float Tau;

    float A;
    float B;

    volatile bool newData;
};

#endif
