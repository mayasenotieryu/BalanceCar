#include <Arduino.h>
#include "IMU.h"
#include "Encodeur.h"
#include "PWM.h"

IMU imu;
Encodeur encodeur;
PWM pwm(600, 570);

// old-platform compatible parameters
float Te = 10.0f;         // ms
float Tau = 1000.0f;     // ms

// control parameters
float Kp_theta = 20.0f;
float Kd_theta = 0.0f;

float Kp_speed = 0.0f;
float Kd_speed = 0.0f;

float theta_eq = 1.56f;

// debug values for the Qt plotting tool
volatile float dbg_theta = 0.0f;
volatile float dbg_gyro  = 0.0f;
volatile float dbg_speed = 0.0f;
volatile float dbg_u     = 0.0f;

void applyCompatibilityUpdate()
{
    imu.setTeMs(Te);
    imu.setTauMs(Tau);
}

void reception(char ch)
{
    static String buffer = "";

    if (ch == '\n' || ch == '\r')
    {
        if (buffer.length() == 0)
            return;

        int spaceIndex = buffer.indexOf(' ');
        if (spaceIndex == -1)
        {
            buffer = "";
            return;
        }

        String cmd   = buffer.substring(0, spaceIndex);
        String value = buffer.substring(spaceIndex + 1);
        float v = value.toFloat();

        if (cmd == "Tau")
        {
            Tau = constrain(v, 1.0f, 10000.0f);
            applyCompatibilityUpdate();
        }
        else if (cmd == "Te")
        {
            Te = constrain(v, 1.0f, 100.0f);
            applyCompatibilityUpdate();
        }
        else if (cmd == "KpT")
        {
            Kp_theta = constrain(v, 0.0f, 100.0f);
        }
        else if (cmd == "KdT")
        {
            Kd_theta = constrain(v, 0.0f, 20.0f);
        }
        else if (cmd == "KpS")
        {
            Kp_speed = constrain(v, 0.0f, 100.0f);
        }
        else if (cmd == "KdS")
        {
            Kd_speed = constrain(v, 0.0f, 20.0f);
        }
        else if (cmd == "theta")
        {
            theta_eq = v;
        }
        else if (cmd == "GL")
        {
            pwm.setGainLeft(v);
        }
        else if (cmd == "GR")
        {
            pwm.setGainRight(v);
        }

        buffer = "";
    }
    else
    {
        buffer += ch;

        if (buffer.length() > 64)
            buffer = "";
    }
}

void controlTask(void *param)
{
    TickType_t lastWake = xTaskGetTickCount();
    float last_speed_error = 0.0f;

    while (1)
    {
        float theta = imu.getAngle();
        float gyro  = imu.getGyroZ();

        encodeur.update();

        float v_left  = encodeur.getSpeed_L();
        float v_right = encodeur.getSpeed_R();
        float v_mean  = 0.5f * (v_left + v_right);

        float Te_s = Te / 1000.0f;

        float speed_error = -v_mean;
        float d_speed = (speed_error - last_speed_error) / Te_s;
        last_speed_error = speed_error;

        float theta_corr =
            Kp_speed * speed_error +
            Kd_speed * d_speed;

        theta_corr = constrain(theta_corr,
                               -2.5f * DEG_TO_RAD,
                                2.5f * DEG_TO_RAD);

        float theta_ref = theta_eq + theta_corr;
        float error_theta = theta_ref - theta;

        float u =
            Kp_theta * error_theta
            - Kd_theta * gyro;

        u = constrain(u, -1000.0f, 1000.0f);

        pwm.setSpeed(-(int)u);

        dbg_theta = theta;
        dbg_gyro  = gyro;
        dbg_speed = v_mean;
        dbg_u     = u;

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS((uint32_t)Te));
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    if (!imu.begin())
    {
        while (1) { delay(10); }
    }

    applyCompatibilityUpdate();
    imu.startTask();

    encodeur.begin();
    pwm.begin();

    xTaskCreate(
        controlTask,
        "control",
        4096,
        NULL,
        5,
        NULL
    );
}

void loop()
{
    while (Serial.available() > 0)
    {
        reception((char)Serial.read());
    }

    // ===== Display scaling factors =====
    static float disp_theta_gain = 0.5f;   // angle display gain
    static float disp_gyro_gain  = 0.5f;   // gyro display gain
    static float disp_speed_gain = 40.0f;  // speed display gain
    static float disp_u_gain     = 0.2f;  // control output display gain

    // ===== Display variables =====
    float theta_plot = dbg_theta * 180.0f / PI * disp_theta_gain; // deg
    float gyro_plot  = dbg_gyro  * 180.0f / PI * disp_gyro_gain;  // scaled deg/s
    float speed_plot = dbg_speed * disp_speed_gain;               // scaled speed
    float u_plot     = dbg_u     * disp_u_gain;                   // scaled control

    Serial.print(theta_plot, 6);
    Serial.print(' ');
    Serial.print(gyro_plot, 6);
    Serial.print(' ');
    Serial.print(speed_plot, 6);
    Serial.print(' ');
    Serial.println(u_plot, 6);

    delay((uint32_t)Te);
}