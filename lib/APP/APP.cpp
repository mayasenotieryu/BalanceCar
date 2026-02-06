#include "APP.h"
#include "UserConfig.h"

const char *ssid = USER_SSID;
const char *password = USER_PASSWORD;

AppControl_t appCTRL;
WebServer appServer(80);

#if STATIC_IP_MODE
IPAddress staticIP(STATIC_IP_FIRST_OCTET, STATIC_IP_SECOND_OCTET,
                   STATIC_IP_THIRD_OCTET, STATIC_IP_FOURTH_OCTET);
IPAddress gateway(GATEWAY_FIRST_OCTET, GATEWAY_SECOND_OCTET,
                  GATEWAY_THIRD_OCTET, GATEWAY_FOURTH_OCTET);
IPAddress subnet(SUBNET_FIRST_OCTET, SUBNET_SECOND_OCTET,
                 SUBNET_THIRD_OCTET, SUBNET_FOURTH_OCTET);
#endif

/* ===================== Web Server Task ===================== */
static void AppServerTask(void *pvParameters)
{
    Serial.begin(115200);

#if STATIC_IP_MODE
    WiFi.config(staticIP, gateway, subnet);
#endif

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("================================");
    Serial.println(WiFi.localIP());
    Serial.println("================================");

    appServer.on("/connect", appConnectHandler);
    appServer.on("/voltage", appVoltageHandler);
    appServer.on("/mpuset", appMPUsetHandler);
    appServer.on("/move", appMoveHandler);

    // 接口保留，但不再做任何硬件操作
    appServer.on("/poweroff", appPowerOffHandler);
    appServer.on("/rgb", appRGBChangeHandler);

    appServer.begin();
    Serial.println("Balance Car AppServer Started");

    for (;;)
    {
        appServer.handleClient();
        vTaskDelay(10);
    }
}

/* ===================== 速度收敛 / 刹车 Task ===================== */
static void CarBrakeTask(void *pvParameters)
{
    for (;;)
    {
        if (appCTRL.Direction == "stop")
        {
            if (appCTRL.Velocity > appCTRL.MPUOffset)
            {
                appCTRL.Velocity -= BRAKE_DECAY;
                if (appCTRL.Velocity < appCTRL.MPUOffset)
                    appCTRL.Velocity = appCTRL.MPUOffset;
            }
            else if (appCTRL.Velocity < appCTRL.MPUOffset)
            {
                appCTRL.Velocity += BRAKE_DECAY;
                if (appCTRL.Velocity > appCTRL.MPUOffset)
                    appCTRL.Velocity = appCTRL.MPUOffset;
            }

            appCTRL.SteerVelocity = 0.0f;
        }
        else if (appCTRL.Direction == "mpu")
        {
            appCTRL.Velocity = appCTRL.MPUOffset;
        }

        // L298N 死区钳制
        if (abs(appCTRL.Velocity) < VEL_DEADZONE)
            appCTRL.Velocity = 0.0f;

        if (abs(appCTRL.SteerVelocity) < STR_DEADZONE)
            appCTRL.SteerVelocity = 0.0f;

        vTaskDelay(10);
    }
}

/* ===================== Task 启动 ===================== */
void AppTaskInit::startTask()
{
    xTaskCreatePinnedToCore(AppServerTask,
                            "App Server Task",
                            6144,
                            NULL,
                            1,
                            NULL,
                            0);

    xTaskCreatePinnedToCore(CarBrakeTask,
                            "Brake Task",
                            2048,
                            NULL,
                            1,
                            NULL,
                            0);
}

/* ===================== Web Handlers ===================== */

static void appConnectHandler()
{
    appServer.send(200, "text/plain", "1");
}

static void appVoltageHandler()
{
    // 没有电池采样硬件，返回占位值
    appServer.send(200, "text/plain", "0");
}

static void appMPUsetHandler()
{
    appCTRL.Direction = appServer.arg("direction");
    appCTRL.MPUOffset = appServer.arg("distance").toFloat();
}

static void appMoveHandler()
{
    appCTRL.Direction = appServer.arg("direction");

    if (appCTRL.Direction == "up")
        appCTRL.Velocity += MOVE_VEL;
    else if (appCTRL.Direction == "down")
        appCTRL.Velocity -= MOVE_VEL;
    else if (appCTRL.Direction == "left")
        appCTRL.SteerVelocity =
            constrain(appCTRL.SteerVelocity + STR_VEL, -STR_LIMIT, STR_LIMIT);
    else if (appCTRL.Direction == "right")
        appCTRL.SteerVelocity =
            constrain(appCTRL.SteerVelocity - STR_VEL, -STR_LIMIT, STR_LIMIT);
}

// 占位接口：不做任何事
static void appPowerOffHandler()
{
    appServer.send(200, "text/plain", "OK");
}

// 占位接口：不做任何事
static void appRGBChangeHandler()
{
    appServer.send(200, "text/plain", "OK");
}

AppTaskInit APP;
