#include "IMU.h"
#include "Encodeur.h"
#include "PWM.h"

// ================= Module Instances =================
IMU imu;
Encodeur encodeur;
PWM pwm(470, 430);   // motor dead zone

// ================= Compatibility Parameters =================
// Keep old platform semantics:
// Te = sampling period in ms
// Tau = filter time constant in ms

float Te = 5.0f;        // ms
float Tau = 1000.0f;    // ms

// ================= Control Parameters =================

// ---- Angle Loop ----
float Kp_theta = 180.0f;
float Kd_theta = 0.2f;

// ---- Speed Loop ----
float Kp_speed = 0.8f;
float Kd_speed = 0.0f;

// ---- Balance Angle ----
float theta_eq = 1.56f;

// ================= Online Tuning System =================

void printParameters()
{
    Serial.println("---- Current Parameters ----");
    Serial.print("Tau(ms): ");   Serial.println(Tau);
    Serial.print("Te(ms): ");    Serial.println(Te);
    Serial.print("alpha: ");     Serial.println(imu.getAlpha(), 6);

    Serial.print("KpT: ");       Serial.println(Kp_theta);
    Serial.print("KdT: ");       Serial.println(Kd_theta);
    Serial.print("KpS: ");       Serial.println(Kp_speed);
    Serial.print("KdS: ");       Serial.println(Kd_speed);
    Serial.print("theta_eq: ");  Serial.println(theta_eq, 6);
    Serial.println("----------------------------");
}

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
        if (buffer.length() == 0) return;

        int spaceIndex = buffer.indexOf(' ');
        String cmd;
        String value;

        if (spaceIndex == -1)
        {
            cmd = buffer;

            if (cmd == "show")
            {
                printParameters();
            }

            buffer = "";
            return;
        }

        cmd   = buffer.substring(0, spaceIndex);
        value = buffer.substring(spaceIndex + 1);
        float v = value.toFloat();

        // ===== Old platform compatible commands =====
        if (cmd == "Tau")
        {
            Tau = v;
            Tau = constrain(Tau, 1.0f, 10000.0f);
            applyCompatibilityUpdate();

            Serial.print("Updated: ");
            Serial.println(buffer);
        }
        else if (cmd == "Te")
        {
            Te = v;
            Te = constrain(Te, 1.0f, 100.0f);
            applyCompatibilityUpdate();

            Serial.print("Updated: ");
            Serial.println(buffer);
        }

        // ===== New extended tuning commands =====
        else if (cmd == "KpT")
        {
            Kp_theta = constrain(v, 0.0f, 500.0f);
            Serial.print("Updated: ");
            Serial.println(buffer);
        }
        else if (cmd == "KdT")
        {
            Kd_theta = constrain(v, 0.0f, 20.0f);
            Serial.print("Updated: ");
            Serial.println(buffer);
        }
        else if (cmd == "KpS")
        {
            Kp_speed = constrain(v, 0.0f, 20.0f);
            Serial.print("Updated: ");
            Serial.println(buffer);
        }
        else if (cmd == "KdS")
        {
            Kd_speed = constrain(v, 0.0f, 20.0f);
            Serial.print("Updated: ");
            Serial.println(buffer);
        }
        else if (cmd == "theta")
        {
            theta_eq = v;
            Serial.print("Updated: ");
            Serial.println(buffer);
        }
        else
        {
            Serial.print("Unknown command: ");
            Serial.println(buffer);
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
    float last_speed_error = 0.0f;

    while (1)
    {
        // 1. Read sensors
        float theta = imu.getAngle();
        float gyro  = imu.getGyroZ();

        encodeur.update();

        float v_left  = encodeur.getSpeed_L();
        float v_right = encodeur.getSpeed_R();
        float v_mean  = 0.5f * (v_left + v_right);

        // 2. Speed loop
        float Te_s = Te / 1000.0f;

        float speed_error = -v_mean;   // target speed = 0
        float d_speed = (speed_error - last_speed_error) / Te_s;
        last_speed_error = speed_error;

        float theta_corr =
            Kp_speed * speed_error +
            Kd_speed * d_speed;

        theta_corr = constrain(theta_corr,
                               -3.0f * DEG_TO_RAD,
                                3.0f * DEG_TO_RAD);

        // 3. Angle loop
        float theta_ref = theta_eq + theta_corr;
        float error_theta = theta_ref - theta;

        float u =
            Kp_theta * error_theta
            - Kd_theta * gyro;

        // 4. Output saturation
        u = constrain(u, -500.0f, 500.0f);

        // 5. Motor output
        pwm.setSpeed(-(int)u);

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS((uint32_t)Te));
    }
}

// ================= Initialization ====================

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println("Balance Control System Ready");

    if (!imu.begin())
    {
        Serial.println("MPU6050 not found!");
        while (1) { delay(10); }
    }

    applyCompatibilityUpdate();
    imu.startTask();

    encodeur.begin();
    pwm.begin();

    xTaskCreate(controlTask,
                "control",
                4096,
                NULL,
                5,
                NULL);

    printParameters();
}

void loop()
{
    while (Serial.available() > 0)
    {
        reception(Serial.read());
    }
}