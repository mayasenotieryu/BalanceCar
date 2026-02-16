#include "IMU.h"
#include <Wire.h>

IMU::IMU()
: tetagr(0), tetaomg(0), tetaF(0),
  Te(5), Tau(1000),
  A(0), B(0),
  newData(false) {}

bool IMU::begin() {

    Wire.begin(21,22);

    if (!mpu.begin())
        return false;

    A = 1 / (1 + Tau / Te);
    B = Tau / Te * A;

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

    tetagr = atan(a.acceleration.x / a.acceleration.y);
    tetaF = A * tetaF + B * tetagr;
    tetaomg = A * Tau / 1000 * g.gyro.z + B * tetaomg;

    newData = true;
}

float IMU::getAngle() {
    return tetaF;
}
