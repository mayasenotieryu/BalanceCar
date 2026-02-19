#include <Arduino.h>

// ================= 电机引脚 =================
#define G_IN1 17
#define G_IN2 18
#define D_IN1 16
#define D_IN2 4

#define CH_G1 0
#define CH_G2 1
#define CH_D1 2
#define CH_D2 3

// ⚠ 方向补偿
#define LEFT_SIGN   -1
#define RIGHT_SIGN  1   

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

void setup() {

    Serial.begin(115200);

    ledcSetup(CH_G1, 20000, 8);
    ledcSetup(CH_G2, 20000, 8);
    ledcSetup(CH_D1, 20000, 8);
    ledcSetup(CH_D2, 20000, 8);

    ledcAttachPin(G_IN1, CH_G1);
    ledcAttachPin(G_IN2, CH_G2);
    ledcAttachPin(D_IN1, CH_D1);
    ledcAttachPin(D_IN2, CH_D2);
}

void loop() {

    setMotor(150);   // 小车应整体向前

    delay(3000);

    setMotor(-150);  // 小车应整体向后

    delay(3000);
}
