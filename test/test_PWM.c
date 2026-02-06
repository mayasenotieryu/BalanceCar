#include <Arduino.h>
#include "SuperCar.h"

/*
 * 这是【纯硬件测试 main】
 * - 不用 APP
 * - 不用 MPU
 * - 不用 PID
 * - 只测试 Mini_L298N + PWM
 */

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("==== Mini_L298N Motor Test Start ====");

    BalanceCar.startTask();
}

void loop()
{
    // 所有测试逻辑都在 SuperCar::running() 里
    BalanceCar.running();
}
