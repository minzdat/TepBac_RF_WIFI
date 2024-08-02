#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <string.h>
#include <esp_mac.h>
#include "esp_timer.h"


#include "connect_wifi.h"
static const char *TAG = "ESP-NOW Sender";


int start=0;
int stop=0;
int time_s=0;

typedef struct {
    // int data;
        char data[250];
        char data2[250];
} esp_now_message_t;

uint8_t lmk[16] = {0x04, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
uint8_t pmk[16] = {0x03, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};

esp_now_peer_info_t peer_info = {};
esp_now_peer_info_t peer_broadcast = {};

// uint8_t mac_a[6] ={0xDC, 0xDA, 0x0C, 0x0D, 0x41, 0xAC};
uint8_t mac_a[6] ={0xDC, 0xDA, 0x0C, 0x0D, 0x41, 0x64};

uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
bool check_en=true;
    uint8_t pair_request[50] ="connect_to_me";
uint8_t mac_s[6];
void recv_callback(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    
// if ((memcmp(recv_info->src_addr, mac_a, 6) == 0)&&(len==50)) {
        // if ((memcmp(data, "Hello from Server!", 10) != 0)&&(len==50)) {
// if ((len==50)) {


    // stop=esp_timer_get_time();
    ESP_LOGW(TAG, "Received data from: " MACSTR ", len: %d", MAC2STR(recv_info->src_addr), len);
    esp_now_message_t *message = (esp_now_message_t *)data;
    ESP_LOGW(TAG, "Received data: %s", message->data);
    //     time_s=stop-start;
    // ESP_LOGE(TAG, "time= %d ms", time_s/1000);
        int8_t rssi = recv_info->rx_ctrl->rssi;
        ESP_LOGI(TAG, "RSSI: %d", rssi);
    if ((memcmp(data,"connect_to_me", sizeof("connect_to_me")) == 0)) {
    if (check_en){
            ESP_LOGE(TAG,"Setup ");
    // mac_s[6]=recv_info->src_addr;
    memcpy(mac_s,recv_info->src_addr, 6);
    memcpy(peer_info.peer_addr, mac_s, 6);
    // memcpy(peer_info.peer_addr, mac_a, 6);
    // peer_info.encrypt = true;
    
    esp_now_add_peer(&peer_info);
    ESP_LOGI(TAG,"Send to "MACSTR" : %s",MAC2STR(mac_s),pair_request);
    esp_now_send(mac_s, pair_request, sizeof(pair_request));
    peer_info.encrypt = true;
    memcpy(peer_info.lmk, lmk, 16);
    check_en=false;
    esp_now_mod_peer(&peer_info);   
     }
    

    }
}

static void send_callback(const uint8_t *mac_addr, esp_now_send_status_t status) {
    printf("Send Status: %s\n", status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");

}
void setup_esp_now_encryption() {
    uint8_t encryption_key[ESP_NOW_KEY_LEN] = { 
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 
    };

    ESP_ERROR_CHECK(esp_now_set_pmk(encryption_key));
}
#define NEW_MAC_ADDRESS "AA:BB:CC:DD:EE:FF"
uint8_t mac_c[6] ={0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

void init_esp_now() {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    esp_wifi_set_channel(3, WIFI_SECOND_CHAN_NONE);
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_now_init());
        // setup_esp_now_encryption();
    // ESP_ERROR_CHECK(esp_now_set_pmk(pmk));
    ESP_ERROR_CHECK(esp_now_register_recv_cb(recv_callback));
    ESP_ERROR_CHECK(esp_now_register_send_cb(send_callback));
    // ESP_ERROR_CHECK(esp_wifi_set_mac(ESP_IF_WIFI_STA, mac_c));
    uint8_t mac[6];
    ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_STA, mac));
    ESP_LOGI(TAG, "MAC Addressssss: "MACSTR,MAC2STR(mac));
}



int count=0;
void set_wifi_max_tx_power(void) {
    esp_err_t err = esp_wifi_set_max_tx_power(20);
    // Kiểm tra và log kết quả
    ESP_LOGE(TAG, "Esp set power: %s", esp_err_to_name(err));
    int8_t tx_power;
    esp_wifi_get_max_tx_power(&tx_power);
    tx_power=tx_power/4;
    ESP_LOGI(TAG, "Current max TX power: %d dBm", tx_power);
}


void app_main(void) {


            ESP_LOGE(TAG,"THiS iS MAINNNNN ");

    // ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init();

    // init_esp_now();

    // wifi_init_sta("Hoai Nam","123456789");
    // set_wifi_max_tx_power();
    // peer_info.encrypt = true;
    // memcpy(peer_info.lmk, lmk, 16);

    //     memcpy(peer_info.peer_addr, mac_a, 6);

    // esp_now_add_peer(&peer_info);
while ((1))
{
    while (!check_en)
    {
        count++;
        uint8_t response[50] ;
        sprintf((char *)response, "Send from esp-now : %d", count);
        ESP_LOGI(TAG,"%s",response);
        esp_now_send(mac_s, response, sizeof(response));
    //     // esp_now_send(broadcast_mac,response, sizeof(response));
        start=esp_timer_get_time();
                vTaskDelay(1000/ portTICK_PERIOD_MS);

    }
        vTaskDelay(1000/ portTICK_PERIOD_MS);

}
}
