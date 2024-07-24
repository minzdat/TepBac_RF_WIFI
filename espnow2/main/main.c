#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <string.h>
#include <esp_mac.h> 
static const char *TAG = "ESP-NOW Receiver";
// uint8_t mac_a[6] = {0xC4, 0xDD, 0x57, 0xC7, 0xDC, 0x58};
uint8_t mac_a[6] ={0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
uint8_t mac_s[6];
// uint8_t mac_r[6] = {0x1A, 0x1E, 0xCA, 0x3F, 0x14, 0x1E};
// uint8_t mac_f[6] = {0x2E, 0x25, 0xCA, 0x3F, 0x28, 0x26};
// static const char* PMK_KEY_STR = "REPLACE_WITH_PMK";
// static const char* LMK_KEY_STR = "REPLACE_WITH_LMK";
// uint8_t lmk[16] = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07, 0x18, 0x29, 0x3A, 0x4B, 0x5C, 0x6D, 0x7E, 0x8F, 0x90};
// uint8_t pmk[16] = {0x6F, 0xA1, 0xB3, 0xC4, 0xD5, 0xE6, 0xF7, 0x08, 0x19, 0x2A, 0x3B, 0x4C, 0x5D, 0x6E, 0x7F, 0x80};

uint8_t lmk[16] = {0x04, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
uint8_t pmk[16] = {0x03, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};

uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

esp_now_peer_info_t peer_info = {};
esp_now_peer_info_t peer_broadcast = {};

typedef struct {
        char data[250]; 
        char data2[250];
} esp_now_message_t;

int count=0;
bool check_en=true;
void setup_esp_now_encryption() {
    uint8_t encryption_key[ESP_NOW_KEY_LEN] = { 
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 
    };

    ESP_ERROR_CHECK(esp_now_set_pmk(encryption_key));
}

void recv_callback(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    // if ((memcmp(data, "Hello from Server!", 10) == 0)) {
    // if ((memcmp(recv_info->src_addr,mac_a,6) ==0)&&(len==50)){
    ESP_LOGW(TAG, "Received data from: " MACSTR ", len: %d", MAC2STR(recv_info->src_addr), len);
        ESP_LOGW(TAG, "des_addr  : " MACSTR ", len: %d", MAC2STR(recv_info->des_addr), len);

    // ESP_LOGW(TAG, "Received data from: " MACSTR ", len: %d", MAC2STR(mac_a), len);
        esp_now_message_t *message = (esp_now_message_t *)data;

    ESP_LOGW(TAG, "Received data: %s", message->data);
    count++;
    uint8_t response[50] ;
    sprintf((char *)response, "Response from Nam: %d", count);
    ESP_LOGI(TAG,"%s",response);

    if ((memcmp(message->data,"connect_to_me", sizeof("connect_to_me")) == 0)) {

    if (check_en){
    // memcpy(peer_info.peer_addr, recv_info->src_addr, 6);
    // memcpy(peer_info.peer_addr, mac_a, 6);
    memcpy(mac_s,recv_info->src_addr, 6);
    memcpy(peer_info.peer_addr, mac_s, 6);
    // peer_info.encrypt = true;
    memcpy(peer_info.lmk, lmk, 16);
    esp_now_add_peer(&peer_info);
    
    esp_now_send(mac_s, response, sizeof(response));
    
    peer_info.encrypt = true;
    check_en=false;
    esp_now_mod_peer(&peer_info);

    }
    }

    // memcpy(mac_s,recv_info->src_addr, 6);
    memcpy(peer_info.peer_addr, recv_info->src_addr, 6);
    esp_now_mod_peer(&peer_info);
    esp_now_send(recv_info->src_addr, response, sizeof(response));
    ESP_LOGI(TAG,"Send to "MACSTR,MAC2STR(recv_info->src_addr));

    // uint8_t mac_b[6] ={0x24, 0x58, 0x7C, 0xF0, 0xB4, 0xD4};




    // else{
    // memcpy(peer_info.peer_addr, recv_info->src_addr, 6);
    // peer_info.encrypt = true;
    // memcpy(peer_info.lmk, encryption_key, 16);
    // // esp_now_add_peer(&peer_info);
    // esp_now_mod_peer(&peer_info);

    // }
    // esp_now_mod_peer(&peer_info);


    // }
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
    ESP_LOGI(TAG, "MAC Address: "MACSTR,MAC2STR(mac));
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    init_esp_now();
memcpy(peer_broadcast.peer_addr, broadcast_mac, 6);
esp_now_add_peer(&peer_broadcast);
while (1){
    // if (check_en){
    uint8_t pair_request[50] ="connect_to_me";
    ESP_LOGI(TAG,"Send to broadcast: %s",pair_request);
    // esp_now_send(broadcast_mac,pair_request,sizeof(pair_request));
    esp_now_send(broadcast_mac,pair_request, sizeof(pair_request));
    vTaskDelay(1500/ portTICK_PERIOD_MS);
}



    // if (addd){
    // memcpy(peer_info.peer_addr, recv_info->src_addr, 6);
    //     memcpy(peer_info.peer_addr, mac_a, 6);

    // peer_info.encrypt = true;
    // memcpy(peer_info.lmk, lmk, 16);
    // esp_now_add_peer(&peer_info);
    // addd=false;
    // }
    // if (addd){

            // setup_esp_now_encryption();
//   esp_now_set_pmk((uint8_t *)PMK_KEY_STR);

    // memcpy(peer_info.peer_addr, mac_a, 6);
    // peer_info.encrypt = true;
    // memcpy(peer_info.lmk, encryption_key, 16);
    // esp_now_add_peer(&peer_info);



    // addd=false;
    // }
    // uint8_t mac_b[6] ={0x24, 0x58, 0x7C, 0xF0, 0xB4, 0xD4};

    // memcpy(peer_info.peer_addr, mac_a, 6);
    // esp_now_add_peer(&peer_info);
    // while (1)
    // {
    // vTaskDelay(3000/ portTICK_PERIOD_MS);
    // esp_now_send(mac_b, data, sizeof(data));
    // }
}

