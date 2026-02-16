#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_netif.h"
#include "mdns.h"

static const char *TAG = "wifi_ap";

/* ================== AP 配置 ================== */
wifi_config_t wifi_config = {
    .ap = {
        .ssid = "zwang",
        .ssid_len = 0,
        .channel = 1,
        .password = "12345678",
        .authmode = WIFI_AUTH_WPA2_PSK,
        .max_connection = 4,
        .pmf_cfg = {.required = false},
    },
};

/* ================== 事件回调声明 ================== */
void on_wifi_ap_start(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data);
void on_client_connected(void *arg, esp_event_base_t event_base,
                         int32_t event_id, void *event_data);
void on_client_disconnected(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data);

/* ================== mDNS 初始化 ================== */
void mdns_setup(void)
{
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set("zwang"));
    ESP_ERROR_CHECK(mdns_instance_name_set("ESP32 SoftAP Device"));

    ESP_LOGI(TAG, "🌐 mDNS 已启动，可通过 http://zwang.local 访问");
}

/* ================== WiFi 初始化 ================== */
void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, WIFI_EVENT_AP_START,
        &on_wifi_ap_start, NULL, NULL));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED,
        &on_client_connected, NULL, NULL));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED,
        &on_client_disconnected, NULL, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "✅ Soft-AP 启动完成");
}

/* ================== 回调函数 ================== */
void on_wifi_ap_start(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "📡 AP 已启动，等待客户端...");

    /* 在 AP 真正启动后再启动 mDNS */
    mdns_setup();
}

void on_client_connected(void *arg, esp_event_base_t event_base,
                         int32_t event_id, void *event_data)
{
    wifi_event_ap_staconnected_t *event =
        (wifi_event_ap_staconnected_t *)event_data;

    ESP_LOGI(TAG, "🟢 新设备连接 - MAC: " MACSTR ", AID: %d",
             MAC2STR(event->mac), event->aid);
}

void on_client_disconnected(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    wifi_event_ap_stadisconnected_t *event =
        (wifi_event_ap_stadisconnected_t *)event_data;

    ESP_LOGI(TAG, "🔴 客户端断开 - MAC: " MACSTR ", AID: %d",
             MAC2STR(event->mac), event->aid);
}

/* ================== 主函数 ================== */
void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NEW_VERSION_DETECTED) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "🚀 启动 ESP32 Soft-AP...");

    wifi_init_softap();
}
