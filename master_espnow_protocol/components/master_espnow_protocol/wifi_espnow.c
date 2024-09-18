#include "master_espnow_protocol.h"

static int s_retry_num = CURRENT_INDEX;
static EventGroupHandle_t s_wifi_event_group;

void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/* WiFi should start before using ESPNOW */
void master_wifi_init(void)
{
/*---------------------------------------------------------------------------------*/
// // Espnow protocol WITH WIFI

//     s_wifi_event_group = xEventGroupCreate();

//     ESP_ERROR_CHECK(esp_netif_init());

//     ESP_ERROR_CHECK(esp_event_loop_create_default());
//     esp_netif_create_default_wifi_sta();

//     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

//     esp_event_handler_instance_t instance_any_id;
//     esp_event_handler_instance_t instance_got_ip;
//     ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
//     ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

//     // Configure Wi-Fi Station mode
//     wifi_config_t wifi_config = {
//         .sta = {
//             .ssid = CONFIG_ESP_WIFI_SSID,
//             .password = CONFIG_ESP_WIFI_PASSWORD,
//             /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (password len => 8).
//              * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
//              * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
//              * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
//              */
//             .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
//             .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
//             .sae_h2e_identifier = MASTER_H2E_IDENTIFIER,
//         },
//     };
//     // ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
//     ESP_ERROR_CHECK( esp_wifi_set_mode(ESPNOW_WIFI_MODE) );
//     ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
//     ESP_ERROR_CHECK( esp_wifi_start());

//     ESP_LOGI(TAG, "wifi_init_sta finished.");

//     /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
//      * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
//     EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
//             WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
//             pdFALSE,
//             pdFALSE,
//             portMAX_DELAY);

//     /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
//      * happened. */
//     if (bits & WIFI_CONNECTED_BIT) {
//         ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
//                 CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);

//         // const char *udp_ip = "34.150.93.172";
//         // unsigned long udp_port = 5010; // 5006 - 5013
//         // udp_logging_init(udp_ip, udp_port, udp_logging_vprintf);

//     } else if (bits & WIFI_FAIL_BIT) {
//         ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
//                  CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
//     } else {
//         ESP_LOGE(TAG, "UNEXPECTED EVENT");
//     }

//     // ESP_ERROR_CHECK( esp_wifi_set_channel(CONFIG_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE));

// #if CONFIG_ESPNOW_ENABLE_LONG_RANGE
//     ESP_ERROR_CHECK( esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) );
// #endif

/*---------------------------------------------------------------------------------*/

    // #if CONFIG_PM_ENABLE
    //     // Configure dynamic frequency scaling:
    //     // maximum and minimum frequencies are set in sdkconfig,
    //     // automatic light sleep is enabled if tickless idle support is enabled.
    //     esp_pm_config_t pm_config = {
    //             .max_freq_mhz = CONFIG_MAX_CPU_FREQ_MHZ,
    //             .min_freq_mhz = CONFIG_MIN_CPU_FREQ_MHZ,
    // #if CONFIG_FREERTOS_USE_TICKLESS_IDLE
    //             .light_sleep_enable = true
    // #endif
    //     };
    //     ESP_ERROR_CHECK( esp_pm_configure(&pm_config) );
    // #endif // CONFIG_PM_ENABLE

/*---------------------------------------------------------------------------------*/
// Espnow protocol NOT WIFI

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(ESPNOW_WIFI_MODE) );
    ESP_ERROR_CHECK( esp_wifi_start());

    // ESP_ERROR_CHECK(esp_wifi_set_inactive_time(WIFI_IF_STA, DEFAULT_BEACON_TIMEOUT));

    // ESP_LOGI(TAG, "esp_wifi_set_ps().");
    // esp_wifi_set_ps(DEFAULT_PS_MODE);

    ESP_ERROR_CHECK( esp_wifi_set_channel(CONFIG_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE));

#if CONFIG_ESPNOW_ENABLE_LONG_RANGE
    ESP_ERROR_CHECK( esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) );
#endif
}