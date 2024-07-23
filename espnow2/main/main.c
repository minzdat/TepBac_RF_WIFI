#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <string.h>
#include <esp_mac.h> 
static const char *TAG = "ESP-NOW Receiver";
// uint8_t mac_a[6] = {0xC4, 0xFD, 0x57, 0xC7, 0xDC, 0x58};
// uint8_t mac_r[6] = {0x1A, 0x1E, 0xCA, 0x3F, 0x14, 0x1E};

// uint8_t mac_f[6] = {0x2E, 0x25, 0xCA, 0x3F, 0x28, 0x26};

typedef struct {
    // int data;
        char data[250]; 
        char data2[250];
 // Chuỗi dữ liệu
} esp_now_message_t;
int count=0;
void recv_callback(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    if ((memcmp(data, "Hello from Server!", 10) != 0)&&(len==50)) {
    esp_now_message_t *message = (esp_now_message_t *)data;
    ESP_LOGW(TAG, "Received data from: " MACSTR ", len: %d", MAC2STR(recv_info->src_addr), len);

    ESP_LOGW(TAG, "Received data: %s", message->data);
    count++;
    uint8_t response[50] ;
    sprintf((char *)response, "Response: %d", count);
    ESP_LOGI(TAG,"%s",response);

    uint8_t mac_b[6] ={0x24, 0x58, 0x7C, 0xF0, 0xB4, 0xD4};
    esp_now_send(mac_b, response, sizeof(response));

    }
}
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

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    init_esp_now();



    uint8_t mac_b[6] ={0x24, 0x58, 0x7C, 0xF0, 0xB4, 0xD4};

    esp_now_peer_info_t peer_info = {};
    memcpy(peer_info.peer_addr, mac_b, 6);
    esp_now_add_peer(&peer_info);
        uint8_t data[] = "Hello ESP-NOW Hoai Nam 2";
    // while (1)
    // {
    // vTaskDelay(3000/ portTICK_PERIOD_MS);
    // esp_now_send(mac_b, data, sizeof(data));
    // }
}

