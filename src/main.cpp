#include <Arduino.h>
#include "IMU.h"
#include "Encodeur.h"
#include "PWM.h"

IMU imu;
Encodeur encodeur;
PWM pwm;

// old-platform compatible parameters
float Te = 5.0f;         // ms
float Tau = 1000.0f;     // ms

// angle-loop parameters
float Kp_theta = 0*0.01;
float Kd_theta = 0.0;

// speed-loop parameters
float Kp_speed = 0.0f;
float Kd_speed = 0.0f;

// equilibrium angle
float theta_eq = 1.56f;

// maximum allowed tilt angle (deg)
float theta_err_deg_abs = 0.0f;
float theta_max_deg = 30.0f;

// EC-layer deadzone compensation for left/right motors
float C0_L = 0.199;
float C0_R = 0.196;

// EC range
float ec = 0.0;
float ec_max = 0.45;
float ec_corr_L = 0.0;
float ec_corr_R = 0.0;

// Motor Control
int motor_cmd_L = 0;
int motor_cmd_R = 0;

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

// =====================================================
// EC compensation
// ec > 0 => ec + C0
// ec < 0 => ec - C0
// ec = 0 => 0
// =====================================================
float ecCompensate(float ec, float C0)
{
    //if (fabs(ec) < 0.0001)
    //    return 0.0;

    if (ec >= 0.0)
        return ec + C0;
    else
        return ec - C0;
}

// =====================================================
// pwmcalcul(ec)
// map EC range [-ec_max, ec_max] to PWM [-1000, 1000]
// =====================================================
int pwmcalcul(float ec)
{
    ec = constrain(ec, -ec_max, ec_max);

    float pwm_value = (ec / ec_max) * 1000.0;
    pwm_value = constrain(pwm_value, -1000.0, 1000.0);

    return (int)pwm_value;
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
            Kp_theta = constrain(v, 0.0f, 0.01);
        }
        else if (cmd == "KdT")
        {
            Kd_theta = constrain(v, 0.0f, 0.005);
        }
        else if (cmd == "KpS")
        {
            Kp_speed = constrain(v, 0.0f, 50.0f);
        }
        else if (cmd == "KdS")
        {
            Kd_speed = constrain(v, 0.0f, 50.0f);
        }
        else if (cmd == "theta")
        {
            theta_eq = v;
        }
        else if (cmd == "Tmax")
        {
            theta_max_deg = constrain(v, 1.0f, 60.0f);
        }
        else if (cmd == "C0L")
        {
            C0_L = constrain(v, 0.0f, 0.5f);
        }
        else if (cmd == "C0R")
        {
            C0_R = constrain(v, 0.0f, 0.5f);
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
        float theta = imu.getAngle();   // rad
        float gyro  = imu.getGyroZ();   // rad/s

        encodeur.update();

        float v_left  = encodeur.getSpeed_L();
        float v_right = encodeur.getSpeed_R();
        float v_mean  = 0.5f * (v_left + v_right);

        // ===== safety tilt limit =====
        theta_err_deg_abs = fabs(theta - theta_eq) * 180.0f / PI;
        if (theta_err_deg_abs > theta_max_deg)   // max 30 degrees
        {
            pwm.stop();

            dbg_theta = theta;
            dbg_gyro  = gyro;
            dbg_speed = v_mean;
            dbg_u     = 0.0f;

            vTaskDelayUntil(&lastWake, pdMS_TO_TICKS((uint32_t)Te));
            continue;
        }

        float Te_s = Te / 1000.0f;

        // ===== speed loop =====
        float speed_error = -v_mean;
        float d_speed = (speed_error - last_speed_error) / Te_s;
        last_speed_error = speed_error;

        float theta_corr =
            (Kp_speed * speed_error) +
            (Kd_speed * d_speed);

        theta_corr = constrain(theta_corr,
                               -3.0f * DEG_TO_RAD,
                                3.0f * DEG_TO_RAD);

        // ===== angle loop =====
        float theta_ref = theta_eq + theta_corr;
        float error_theta = theta_ref - theta;   // rad

        float error_theta_deg = error_theta * 180.0f / PI;
        float gyro_deg = gyro * 180.0f / PI;

        // raw controller output at EC layer
        ec =
            (Kp_theta * error_theta_deg)
            - (Kd_theta * gyro_deg);

        // limit EC
        ec = constrain(ec, -ec_max, ec_max);

        // independent EC compensation for left/right motors
        ec_corr_L = ecCompensate(ec, C0_L);
        ec_corr_R = ecCompensate(ec, C0_R);

        // unified mapping for each side
        motor_cmd_L = pwmcalcul(ec_corr_L);
        motor_cmd_R = pwmcalcul(ec_corr_R);

        pwm.setSpeedLR(-motor_cmd_L, -motor_cmd_R);

        dbg_theta = theta;
        dbg_gyro  = gyro;
        dbg_speed = v_mean;
        dbg_u     = 0.5f * (motor_cmd_L + motor_cmd_R);

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

    static float disp_theta_gain = 0.5f;
    static float disp_gyro_gain  = 0.5f;
    static float disp_speed_gain = 40.0f;
    static float disp_u_gain     = 0.2f;

    // ===== Display variables =====
    float theta_plot = dbg_theta * 180.0f / PI * disp_theta_gain;
    float gyro_plot  = dbg_gyro  * 180.0f / PI * disp_gyro_gain;
    float speed_plot = dbg_speed * disp_speed_gain;
    float u_plot     = dbg_u     * disp_u_gain;

    Serial.print(theta_plot, 6);
    Serial.print(' ');
    Serial.print(gyro_plot, 6);
    Serial.print(' ');
    Serial.print(speed_plot, 6);
    Serial.print(' ');
    Serial.println(motor_cmd_L);

    delay((uint32_t)Te);
}