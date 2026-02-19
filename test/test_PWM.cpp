#include <Arduino.h>
#include "IMU.h"
#include "Encodeur.h"

// ================= 电机引脚 =================
#define G_IN1 17
#define G_IN2 18
#define D_IN1 16
#define D_IN2 4

#define CH_G1 0
#define CH_G2 1
#define CH_D1 2
#define CH_D2 3

#define LEFT_SIGN   1
#define RIGHT_SIGN -1

// ================= 控制参数 =================
float Kp = 25.0;     // 先偏大一点
float Kd = 1.2;      // 阻尼增强

// 直立平衡角（你测得）
float theta_eq = 1.56;

IMU imu;
Encodeur encodeur;   // 暂时不用，但保留

// ================= 电机底层驱动 =================
void driveLeftRaw(int duty) {
    if (duty >= 0) {
        ledcWrite(CH_G1, duty);
        ledcWrite(CH_G2, 0);
    } else {
        ledcWrite(CH_G1, 0);
        ledcWrite(CH_G2, -duty);
    }
}

void driveRightRaw(int duty) {
    if (duty >= 0) {
        ledcWrite(CH_D1, duty);
        ledcWrite(CH_D2, 0);
    } else {
        ledcWrite(CH_D1, 0);
        ledcWrite(CH_D2, -duty);
    }
}

void setMotor(int pwm) {

    pwm = constrain(pwm, -255, 255);

    int left  = LEFT_SIGN  * pwm;
    int right = RIGHT_SIGN * pwm;

    driveLeftRaw(left);
    driveRightRaw(right);
}

void motorStop() {
    setMotor(0);
}

// ================= 初始化 =================
void setup() {

    Serial.begin(115200);

    // IMU
    if (!imu.begin()) {
        Serial.println("MPU6050 not found!");
        while(1);
    }

    imu.startTask();

    // PWM
    ledcSetup(CH_G1, 20000, 8);
    ledcSetup(CH_G2, 20000, 8);
    ledcSetup(CH_D1, 20000, 8);
    ledcSetup(CH_D2, 20000, 8);

    ledcAttachPin(G_IN1, CH_G1);
    ledcAttachPin(G_IN2, CH_G2);
    ledcAttachPin(D_IN1, CH_D1);
    ledcAttachPin(D_IN2, CH_D2);

    Serial.println("Balance Test Ready");
}

// ================= 主循环 =================
void loop() {

    float angle = imu.getAngle();      // rad
    float gyro  = imu.getGyroZ();      // 你已经改成Y轴

    // ===== 平衡误差 =====
    float error = angle - theta_eq;

    // ===== PD 控制 =====
    float pwmFloat = -Kp * error - Kd * gyro;
    int pwm = (int)pwmFloat;

    // ===== 安全保护 =====
    if (abs(error) > 0.8) {   // 允许较大范围
        motorStop();
        Serial.println("FALL DETECTED");
    } else {
        setMotor(pwm);
    }

    // 调试
    Serial.print("Angle: ");
    Serial.print(angle);
    Serial.print("  Error: ");
    Serial.print(error);
    Serial.print("  Gyro: ");
    Serial.print(gyro);
    Serial.print("  PWM: ");
    Serial.println(pwm);

    delay(5);  // 200Hz
}
