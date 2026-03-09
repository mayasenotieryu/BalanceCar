#include "wifi_ap.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "mdns.h"

#include <string.h>

static const char *TAG = "wifi_ap";

/* ================= AP Para ================= */

wifi_config_t wifi_config;

/* ================= mDNS ================= */

void mdns_setup()
{
    mdns_init();
    mdns_hostname_set("zwang");
    mdns_instance_name_set("ESP32 SoftAP Device");

    ESP_LOGI(TAG, "mDNS started: http://zwang.local");
}

/* ================= Fonction ================= */

void on_wifi_ap_start(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "AP started");

    mdns_setup();
}

void on_client_connected(void *arg, esp_event_base_t event_base,
                         int32_t event_id, void *event_data)
{
    wifi_event_ap_staconnected_t *event =
        (wifi_event_ap_staconnected_t *)event_data;

    ESP_LOGI(TAG,
             "Client connected MAC:" MACSTR " AID:%d",
             MAC2STR(event->mac),
             event->aid);
}

void on_client_disconnected(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    wifi_event_ap_stadisconnected_t *event =
        (wifi_event_ap_stadisconnected_t *)event_data;

    ESP_LOGI(TAG,
             "Client disconnected MAC:" MACSTR " AID:%d",
             MAC2STR(event->mac),
             event->aid);
}

/* ================= WiFi init ================= */

void wifi_init_softap()
{
    esp_netif_init();
    esp_event_loop_create_default();

    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    memset(&wifi_config, 0, sizeof(wifi_config));

    strcpy((char *)wifi_config.ap.ssid, "zwang");
    strcpy((char *)wifi_config.ap.password, "12345678");

    wifi_config.ap.channel = 1;
    wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.ap.max_connection = 4;

    esp_event_handler_instance_register(
        WIFI_EVENT,
        WIFI_EVENT_AP_START,
        &on_wifi_ap_start,
        NULL,
        NULL);

    esp_event_handler_instance_register(
        WIFI_EVENT,
        WIFI_EVENT_AP_STACONNECTED,
        &on_client_connected,
        NULL,
        NULL);

    esp_event_handler_instance_register(
        WIFI_EVENT,
        WIFI_EVENT_AP_STADISCONNECTED,
        &on_client_disconnected,
        NULL,
        NULL);

    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "SoftAP started");
}