#include "IMU.h"
#include "Encodeur.h"
#include "PWM.h"

// ================= Module Instances =================
IMU imu;
Encodeur encodeur;
PWM pwm(470, 430);   // motor dead zone

// ================= Control Parameters =================

// ---- Angle Loop ----
float Kp_theta = 180.0;
float Kd_theta = 0.2;

// ---- Speed Loop ----
float Kp_speed = 0.8;
float Kd_speed = 0.0;

// ---- Balance Angle ----
float theta_eq = 1.56;  // upright equilibrium angle 

// ---- Sampling Period ----
float Te = 0.005;   // 5 ms

// ================= Online Tuning System ==============

void reception(char ch)
{
    static String buffer = "";

    if (ch == '\n' || ch == '\r')
    {
        if (buffer.length() == 0) return;

        int spaceIndex = buffer.indexOf(' ');
        String cmd;
        String value;

        if (spaceIndex != -1)
        {
            cmd   = buffer.substring(0, spaceIndex);
            value = buffer.substring(spaceIndex + 1);
            float v = value.toFloat();

            if (cmd == "KpT")   Kp_theta = v;
            if (cmd == "KdT")   Kd_theta = v;
            if (cmd == "KpS")   Kp_speed = v;
            if (cmd == "KdS")   Kd_speed = v;
            if (cmd == "theta") theta_eq = v;

            // Safety saturation for parameters
            Kp_theta = constrain(Kp_theta, 0, 500);
            Kd_theta = constrain(Kd_theta, 0, 10);
            Kp_speed = constrain(Kp_speed, 0, 10);
            Kd_speed = constrain(Kd_speed, 0, 10);

            Serial.print("Updated: ");
            Serial.println(buffer);
        }
        else if (buffer == "show")
        {
            Serial.println("---- Current Parameters ----");
            Serial.print("KpT: "); Serial.println(Kp_theta);
            Serial.print("KdT: "); Serial.println(Kd_theta);
            Serial.print("KpS: "); Serial.println(Kp_speed);
            Serial.print("KdS: "); Serial.println(Kd_speed);
            Serial.print("theta_eq: "); Serial.println(theta_eq);
            Serial.println("----------------------------");
        }

        buffer = "";
    }
    else
    {
        buffer += ch;
    }
}

// ================= Control Task ======================

void controlTask(void *param)
{
    TickType_t lastWake = xTaskGetTickCount();
    float last_speed_error = 0;

    while (1)
    {
        // ========= 1. Read Sensors =========

        float theta = imu.getAngle();
        float gyro  = imu.getGyroZ();

        encodeur.update();

        float v_left  = encodeur.getSpeed_L();
        float v_right = encodeur.getSpeed_R();
        float v_mean  = 0.5 * (v_left + v_right);

        // ========= 2. Speed Loop =========

        float speed_error = -v_mean;   // target speed = 0

        float d_speed = (speed_error - last_speed_error) / Te;
        last_speed_error = speed_error;

        float theta_corr =
            Kp_speed * speed_error +
            Kd_speed * d_speed;

        // Limit correction to ±3 degrees
        theta_corr = constrain(theta_corr,
                               -3.0 * DEG_TO_RAD,
                                3.0 * DEG_TO_RAD);

        // ========= 3. Angle Loop =========

        float theta_ref = theta_eq + theta_corr;

        float error_theta = theta_ref - theta;

        float u =
            Kp_theta * error_theta
            - Kd_theta * gyro;

        // ========= 4. Output Saturation =========

        u = constrain(u, -500, 500);

        // ========= 5. Motor Output =========

        pwm.setSpeed(-(int)u);

        vTaskDelayUntil(&lastWake,
                        pdMS_TO_TICKS(5));
    }
}

// ================= Initialization ====================

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println("Balance Control System Ready");

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
    while (Serial.available() > 0)
    {
        reception(Serial.read());
    }
}