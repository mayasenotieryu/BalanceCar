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

    float getAngle();      // 融合角度
    float getAccAngle();   // 加速度角
    float getGyroZ();      // 陀螺Z轴角速度

private:
    static void taskWrapper(void *param);
    void update();

    Adafruit_MPU6050 mpu;

    float tetagr;      // 加速度角
    float tetaF;       // 融合角

    float lastGyroZ;   // 最近一次陀螺Z轴数据

    float Te;          // 采样周期 ms
    float alpha;       // 互补滤波系数

    volatile bool newData;
};

#endif
