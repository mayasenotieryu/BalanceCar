#include <Arduino.h>
#include "IMU.h"
#include "Encodeur.h"
#include "PWM.h"
#include "webserver.h"
#include "wifi_ap.h"

volatile float web_angle = 0;
volatile float web_speed = 0;

// ================= Object =================
IMU imu;
Encodeur encodeur;
PWM pwm(470, 430);

// ================= Balance Para =================
float Kp_theta = 180.0;
float Kd_theta = 0.2;

float Kp_speed = 0.8;
float Kd_speed = 0.0;

float theta_eq = 1.56;

float Te = 0.005;

// ================= Web var =================
volatile int joy_x = 0;
volatile int joy_y = 0;

// ================= ControlTask =================
void controlTask(void *param)
{
    TickType_t lastWake = xTaskGetTickCount();

    float last_speed_error = 0;

    while (1)
    {
        float theta = imu.getAngle();
        float gyro  = imu.getGyroZ();

        encodeur.update();

        float v_left  = encodeur.getSpeed_L();
        float v_right = encodeur.getSpeed_R();
        float v_mean  = 0.5 * (v_left + v_right);

        // ===== 摇杆控制速度 =====
        float v_cons = joy_y * 0.01;

        float speed_error = v_cons - v_mean;

        float d_speed = (speed_error - last_speed_error) / Te;
        last_speed_error = speed_error;

        float theta_corr =
            Kp_speed * speed_error +
            Kd_speed * d_speed;

        theta_corr = constrain(theta_corr,
                               -3.0 * DEG_TO_RAD,
                                3.0 * DEG_TO_RAD);

        float theta_ref = theta_eq + theta_corr;

        float error_theta = theta_ref - theta;

        float u =
            Kp_theta * error_theta
            - Kd_theta * gyro;

        u = constrain(u, -500, 500);

        pwm.setSpeed(-(int)u);
        
        web_angle = theta;
        web_speed = v_mean*100;
        
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(5));
    }
}

// ================= Init =================
void setup()
{
    Serial.begin(115200);

    imu.begin();
    imu.startTask();

    encodeur.begin();
    pwm.begin();
    
    wifi_init_softap();
    start_webserver(&joy_x, &joy_y);

    xTaskCreate(controlTask,
                "control",
                4096,
                NULL,
                5,
                NULL);
}

void loop()
{

}