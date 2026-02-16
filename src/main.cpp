#include <Arduino.h>
#include "IMU.h"
#include "Encoder.h"

// ================= 电机引脚 =================
#define G_IN1 17
#define G_IN2 18
#define D_IN1 16
#define D_IN2 4

#define CH_G1 0
#define CH_G2 1
#define CH_D1 2
#define CH_D2 3

// 方向补偿（根据你之前结构）
#define LEFT_SIGN   1
#define RIGHT_SIGN -1

// ================= 控制参数 =================
float Kp = 18.0;     // 先小一点
float Kd = 0.6;      // 阻尼

IMU imu;
Encoder encoder;

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

    // IMU 初始化
    if (!imu.begin()) {
        Serial.println("MPU6050 not found!");
        while(1);
    }

    imu.startTask();

    // PWM 初始化
    ledcSetup(CH_G1, 20000, 8);
    ledcSetup(CH_G2, 20000, 8);
    ledcSetup(CH_D1, 20000, 8);
    ledcSetup(CH_D2, 20000, 8);

    ledcAttachPin(G_IN1, CH_G1);
    ledcAttachPin(G_IN2, CH_G2);
    ledcAttachPin(D_IN1, CH_D1);
    ledcAttachPin(D_IN2, CH_D2);

    Serial.println("Balance test ready");
}

// ================= 主循环 =================
void loop() {

    // 读取姿态
    float angle = imu.getAngle();      // rad
    float gyro  = imu.getGyroZ();      // rad/s

    // ===== PD 控制 =====
    float pwmFloat = -Kp * angle - Kd * gyro;
    int pwm = (int)pwmFloat;

    // ===== 安全保护 =====
    if (abs(angle) > 0.7) {   // ≈ 40°
        motorStop();
        Serial.println("Angle limit exceeded");
    } else {
        setMotor(pwm);
    }

    // 调试输出
    Serial.print("Angle(rad): ");
    Serial.print(angle);
    Serial.print("  Gyro(rad/s): ");
    Serial.print(gyro);
    Serial.print("  PWM: ");
    Serial.println(pwm);

    delay(5);  // 与 IMU 200Hz 同步
}
