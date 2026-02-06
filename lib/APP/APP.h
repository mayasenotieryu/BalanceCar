#ifndef APP_h
#define APP_h

#include <UserConfig.h>
#include <WebServer.h>
#include <WiFi.h>

/****************************** IP 配置 ***********************************/
#define STATIC_IP_FIRST_OCTET 192
#define STATIC_IP_SECOND_OCTET 168
#define STATIC_IP_THIRD_OCTET YOUR_IP
#define STATIC_IP_FOURTH_OCTET 80

#define GATEWAY_FIRST_OCTET 192
#define GATEWAY_SECOND_OCTET 168
#define GATEWAY_THIRD_OCTET 221
#define GATEWAY_FOURTH_OCTET 78

#define SUBNET_FIRST_OCTET 255
#define SUBNET_SECOND_OCTET YOUR_IP
#define SUBNET_THIRD_OCTET 255
#define SUBNET_FOURTH_OCTET 0
/*********************************************************************/

/* ================== 针对 L298N 的 APP 层参数 ================== */
#define BRAKE_DECAY   15.0f

#define MOVE_VEL     4.0f
#define STR_VEL      2.0f

#define VEL_DEADZONE 10.0f
#define STR_DEADZONE 5.0f
/* ============================================================= */

typedef struct appControl
{
    String Direction = "stop";

    // 平衡相关
    volatile float MPUOffset = 0.0f;

    // 给下层控制用的“期望量”
    volatile float Velocity = 0.0f;
    volatile float SteerVelocity = 0.0f;

} AppControl_t;

/* Web 回调 */
static void appConnectHandler();
static void appVoltageHandler();
static void appMPUsetHandler();
static void appMoveHandler();
static void appRGBChangeHandler();   // 保留接口，占位
static void appPowerOffHandler();    // 保留接口，占位

class AppTaskInit
{
  public:
    void startTask();
};

extern AppTaskInit APP;
extern AppControl_t appCTRL;

#endif
