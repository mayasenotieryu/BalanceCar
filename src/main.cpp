#include "IMU.h"
#include "Encodeur.h"
#include "PWM.h"

// ================= 模块实例 =================
IMU imu;
Encodeur encodeur;
PWM pwm(470, 430);   // dead zone

// ================= 平衡参数 =================

// ---- 角度环 ----
float Kp_theta = 180.0;
float Kd_theta = 0.2;

// ---- 速度环 ----
float Kp_speed = 0.8;
float Kd_speed = 0.0;

// ---- 平衡角 ----
float theta_eq = 1.56;  // 直立角

// ---- 采样周期 ----
float Te = 0.005;   // 5ms

// ================= 控制任务 =================
void controlTask(void *param)
{
    TickType_t lastWake = xTaskGetTickCount();

    float last_speed_error = 0;

    while (1)
    {
        // ========= 1. 读取传感器 =========

        float theta = imu.getAngle();
        float gyro  = imu.getGyroZ();

        encodeur.update();

        float v_left  = encodeur.getSpeed_L();
        float v_right = encodeur.getSpeed_R();
        float v_mean  = 0.5 * (v_left + v_right);

        // ========= 2. 速度环 =========

        float speed_error = -v_mean;   // v_cons = 0

        float d_speed = (speed_error - last_speed_error) / Te;
        last_speed_error = speed_error;

        float theta_corr =
            Kp_speed * speed_error +
            Kd_speed * d_speed;

        // 限制在 ±3°
        theta_corr = constrain(theta_corr,
                               -3.0 * DEG_TO_RAD,
                                3.0 * DEG_TO_RAD);

        // ========= 3. 角度环 =========

        float theta_ref = theta_eq + theta_corr;

        float error_theta = theta_ref - theta;

        float u =
            Kp_theta * error_theta
            - Kd_theta * gyro;

        // ========= 4. 饱和 =========

        u = constrain(u, -500, 500);

        // ========= 5. 输出 =========

        pwm.setSpeed(-(int)u);

        vTaskDelayUntil(&lastWake,
                        pdMS_TO_TICKS(5));
    }
}

// ================= 初始化 =================
void setup()
{
    Serial.begin(115200);

    imu.begin();
    imu.startTask();

    encodeur.begin();
    pwm.begin();

    xTaskCreate(controlTask,
                "control",
                4096,
                NULL,
                5,
                NULL);
}

void loop()
{
    // 空
}