#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <string.h>
#include <esp_mac.h>
#include "esp_timer.h"

static const char *TAG = "ESP-NOW Sender";


int start=0;
int stop=0;
int time_s=0;

typedef struct {
    // int data;
        char data[250];
        char data2[250];
} esp_now_message_t;

// typedef struct esp_now_recv_info {
//     uint8_t * src_addr;                      /**< Source address of ESPNOW packet */
//     uint8_t * des_addr;                      /**< Destination address of ESPNOW packet */
//     wifi_pkt_rx_ctrl_t * rx_ctrl;            /**< Rx control info of ESPNOW packet */
// } esp_now_recv_info_t;

void recv_callback(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    
        // if ((memcmp(data, "Hello from Server!", 10) != 0)&&(len==50)) {
    stop=esp_timer_get_time();
    ESP_LOGW(TAG, "Received data from: " MACSTR ", len: %d", MAC2STR(recv_info->src_addr), len);
    esp_now_message_t *message = (esp_now_message_t *)data;
    ESP_LOGW(TAG, "Received data: %s", message->data);
        time_s=stop-start;
    ESP_LOGE(TAG, "time= %d ms", time_s/1000);
    // ESP_LOGI(TAG, "Received Data: %.*s\n", data_len, data);
    // }
}
// void recv_callback(const uint8_t *mac_addr, const uint8_t *data, int len) {
//         if ((memcmp(data, "Hello from Server!", 10) != 0)&&(len==50)) {
//     stop=esp_timer_get_time();
//     // ESP_LOGW(TAG, "Received data from: " MACSTR ", len: %d", MAC2STR(mac_addr), len);
//     esp_now_message_t *message = (esp_now_message_t *)data;
//     ESP_LOGW(TAG, "Received data: %s", message->data);
//         time_s=stop-start;
//     ESP_LOGE(TAG, "time= %d ms", time_s/1000);
//     // ESP_LOGI(TAG, "Received Data: %.*s\n", data_len, data);
//     }
// }
static void send_callback(const uint8_t *mac_addr, esp_now_send_status_t status) {
    printf("Send Status: %s\n", status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");

}

void init_esp_now() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    esp_wifi_set_channel(7, WIFI_SECOND_CHAN_NONE);
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(recv_callback));
    ESP_ERROR_CHECK(esp_now_register_send_cb(send_callback));
    uint8_t mac[6];
    ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_STA, mac));
    ESP_LOGI(TAG, "MAC Address: %02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

}
int count=0;
void set_wifi_max_tx_power(void) {
    esp_err_t err = esp_wifi_set_max_tx_power(8);
    // Kiểm tra và log kết quả
    ESP_LOGE(TAG, "Esp set power: %s", esp_err_to_name(err));

    int8_t tx_power;
    esp_wifi_get_max_tx_power(&tx_power);
    tx_power=tx_power/4;
    ESP_LOGI(TAG, "Current max TX power: %d dBm", tx_power);
}
void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    init_esp_now();
    set_wifi_max_tx_power();


    // uint8_t mac_a[6] ={0x34, 0x85, 0x18, 0x03, 0x95, 0x08}; // MAC của thiết bị nhận
    uint8_t mac_a[6] ={0xDC, 0xDA, 0x0C, 0x0D, 0x41, 0xAC};
    esp_now_peer_info_t peer_info = {};
    memcpy(peer_info.peer_addr, mac_a, 6);
    esp_now_add_peer(&peer_info);
    while (1)
    {
    count++;
    uint8_t response[50] ;
    sprintf((char *)response, "Send from esp-now : %d", count);
    ESP_LOGI(TAG,"%s",response);
    esp_now_send(mac_a, response, sizeof(response));
    start=esp_timer_get_time();
    vTaskDelay(300/ portTICK_PERIOD_MS);

    }
}
