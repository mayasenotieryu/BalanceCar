#include "IMU.h"
#include <Wire.h>

IMU::IMU()
: tetagr(0),
  tetaF(0),
  lastGyroZ(0),
  Te(5),          // 5ms采样周期
  alpha(0.98),    // 互补滤波系数
  newData(false)
{}

bool IMU::begin() {

    Wire.begin(21, 22);  // ESP32 I2C 

    if (!mpu.begin())
        return false;

    // 设置陀螺范围
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);

    // 设置加速度范围
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);

    // 设置低通滤波
    mpu.setFilterBandwidth(MPU6050_BAND_44_HZ);

    return true;
}

void IMU::startTask() {

    xTaskCreate(
        taskWrapper,
        "IMU_Task",
        4096,
        this,
        5,
        NULL
    );
}

void IMU::taskWrapper(void *param) {

    IMU *imu = static_cast<IMU*>(param);
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) {

        imu->update();

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(imu->Te));
    }
}

void IMU::update() {

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // 1️ 加速度计算角度
    tetagr = atan2(a.acceleration.x, a.acceleration.y);

    // 2️ 计算时间步长
    float dt = Te / 1000.0;

    // 3️ 保存陀螺数据
    lastGyroZ = g.gyro.z;

    // 4️ 陀螺积分
    float gyroAngle = tetaF + lastGyroZ * dt;

    // 5️ 互补滤波融合
    tetaF = alpha * gyroAngle + (1.0 - alpha) * tetagr;

    newData = true;
}

float IMU::getAngle() {
    return tetaF;
}

float IMU::getAccAngle() {
    return tetagr;
}

float IMU::getGyroZ() {
    return lastGyroZ;
}
