#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <string.h>
#include <esp_mac.h>
#include "esp_timer.h"
#include <math.h>


#include "connect_wifi.h"
#include "pub_sub_client.h"

static const char *TAG = "ESP-NOW Master";


int start=0;
int stop=0;
int time_s=0;

typedef struct {
        char data[250];
} esp_now_message_t;

uint8_t lmk[16] = {0x04, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
uint8_t pmk[16] = {0x03, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

esp_now_peer_info_t peer_info = {};
esp_now_peer_info_t peer_broadcast = {};

int8_t rssi;
// uint8_t mac_a[6] ={0xDC, 0xDA, 0x0C, 0x0D, 0x41, 0xAC};
// uint8_t mac_a[6] ={0xDC, 0xDA, 0x0C, 0x0D, 0x41, 0x64};

bool check_en=true;
uint8_t pair_request[] ="connect_to_me";
uint8_t mac_s[6];
int num_data=0;

void extract_number( char *input, int *number);

void recv_callback(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
// if ((memcmp(recv_info->src_addr, mac_a, 6) == 0)&&(len==50)) {
        // if ((memcmp(data, "Hello from Server!", 10) != 0)&&(len==50)) {
    // stop=esp_timer_get_time();
    ESP_LOGW(TAG, "Received data from: " MACSTR ", len: %d", MAC2STR(recv_info->src_addr), len);
    esp_now_message_t *message = (esp_now_message_t *)data;
    ESP_LOGW(TAG, "Received data: %s", message->data);

    extract_number(message->data, &num_data);
    // ESP_LOGE(TAG, "NUMBER: %d", num_data);
    // time_s=stop-start;
    // ESP_LOGE(TAG, "time= %d ms", time_s/1000);
    rssi = recv_info->rx_ctrl->rssi;
    ESP_LOGI(TAG, "RSSI: %d", rssi);

    if ((memcmp(data,pair_request, sizeof(pair_request)) == 0)) {
    if (check_en){
    ESP_LOGE(TAG,"Setup ");
    memcpy(mac_s,recv_info->src_addr, 6);
    memcpy(peer_info.peer_addr, mac_s, 6);
    // peer_info.ifidx=ESP_IF_WIFI_STA;
    // memcpy(peer_info.peer_addr, mac_a, 6);    
    esp_now_add_peer(&peer_info);//setup peer
    ESP_LOGI(TAG,"Send to "MACSTR" : %s",MAC2STR(mac_s),pair_request);
    esp_now_send(mac_s, pair_request, sizeof(pair_request));

    peer_info.encrypt = true;
    memcpy(peer_info.lmk, lmk, 16);//add lmk
    esp_now_mod_peer(&peer_info);   

    check_en=false;
    }
    }
}

void extract_number( char *input, int *number) {
    // Tìm vị trí của dấu ':' trong chuỗi
    const char *colon_pos = strchr(input, ':');
    if (colon_pos != NULL) {
        // Chuyển con trỏ tới vị trí sau dấu ':' và bỏ qua khoảng trắng
        colon_pos++; // Bỏ qua dấu ':'
        while (*colon_pos == ' ') {
            colon_pos++; // Bỏ qua các khoảng trắng
        }
        // Chuyển đổi phần chuỗi còn lại thành số nguyên
        *number = atoi(colon_pos);
    } else {
        // Nếu không tìm thấy dấu ':', đặt number bằng 0 hoặc giá trị mặc định khác
        *number = 0;
    }
}

static void send_callback(const uint8_t *mac_addr, esp_now_send_status_t status) {
    printf("Send Status: %s\n", status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

uint8_t mac_c[6] ={0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
// uint8_t mac_c[6] ={0x4A, 0x77, 0xd0, 0x7b, 0x87, 0xAF};

// void wifi_init_esp_now() {
//     // wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     // ESP_ERROR_CHECK(esp_wifi_init(&cfg));
//     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
//     // esp_wifi_set_channel(3, WIFI_SECOND_CHAN_NONE);
//     ESP_ERROR_CHECK(esp_wifi_start());
//     ESP_ERROR_CHECK(esp_now_init());
//     // ESP_ERROR_CHECK(esp_now_set_pmk(pmk));
//     ESP_ERROR_CHECK(esp_now_register_recv_cb(recv_callback));
//     ESP_ERROR_CHECK(esp_now_register_send_cb(send_callback));
//     // ESP_ERROR_CHECK(esp_wifi_set_mac(ESP_IF_WIFI_STA, mac_c));
//     uint8_t mac[6];
//     ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_STA, mac));
//     ESP_LOGE(TAG, "MAC Address: "MACSTR,MAC2STR(mac));
// } //for esp-now



int count=0;


static void esp_send_now(void *pvParameters){
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


static void mqtt_task(void *pvParameters)
{
    char data[100];
    // SensorsData sensor;
    BaseType_t ret;
    float angle=-100;
    float sin_angle;
    float sin_angle2;
    float sin_angle3;
    float sin_angle4;
    while(1)
    {
        // ret=xQueueReceive(g_publisher_queue,&sensor,(TickType_t)portMAX_DELAY);
        if(true)
        {
            ESP_LOGI(TAG,"Receive data from queue successfully");
            sin_angle = sin(angle*0.06283)*10+10;
            sin_angle2 = sin(angle*0.031415)*10+10;
            sin_angle3 = sin(angle*0.0157)*10+10;

            angle++;
            if (angle>100){
                angle=-100;
            }
            sprintf(data, "temperature_rdo: %f, do: %f, temperature_phg: %f, ph: %d",sin_angle,sin_angle2,sin_angle3,num_data);
            // xEventGroupWaitBits(g_wifi_event, g_constant_wifi_connected_bit, pdFALSE, pdTRUE, portMAX_DELAY);
            //g_index_queue=0;
            data_to_mqtt(data, "v1/devices/me/telemetry",500, 1);
            
                    vTaskDelay(100/ portTICK_PERIOD_MS);

        }
    }
    vTaskDelete(NULL);
}


void wifi_init_esp_now() {
    // ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK(esp_event_loop_create_default());
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
    ESP_ERROR_CHECK(esp_wifi_set_mac(ESP_IF_WIFI_STA, mac_c));
    uint8_t mac[6];
    ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_STA, mac));
    ESP_LOGI(TAG, "MAC Address: "MACSTR,MAC2STR(mac));
}
// void wifi_init_esp_now() {
//     // wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     // ESP_ERROR_CHECK(esp_wifi_init(&cfg));
//     // ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
//     // esp_wifi_set_channel(3, WIFI_SECOND_CHAN_NONE);
//     ESP_ERROR_CHECK(esp_now_init());
//     ESP_ERROR_CHECK(esp_wifi_start());

//     // ESP_ERROR_CHECK(esp_now_set_pmk(pmk));
//     ESP_ERROR_CHECK(esp_now_register_recv_cb(recv_callback));
//     ESP_ERROR_CHECK(esp_now_register_send_cb(send_callback));
//     // ESP_ERROR_CHECK(esp_wifi_set_mac(ESP_IF_WIFI_STA, mac_c));
//     uint8_t mac[6];
//     ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_STA, mac));
//     ESP_LOGE(TAG, "MAC Address: "MACSTR,MAC2STR(mac));
// } //for esp-now

#define SSID "Hoai Nam"
#define PASS "123456789"
#define BROKER "mqtt://35.240.204.122:1883"
#define USER_NAME "tts-3"
#define TOPIC "v1/devices/me/rpc/request/+"
#define MAX_RSSI 20
void app_main(void) {

    wifi_init();
            ESP_LOGI(TAG,"INIT NOW");
            ESP_LOGI(TAG,"INIT wifi_init_sta");
                    wifi_init_sta(SSID,PASS);
                wifi_init_esp_now();

        // vTaskDelay(5000/ portTICK_PERIOD_MS);

        // ESP_ERROR_CHECK(esp_now_init());

                // wifi_init_esp_now();
                //     wifi_init_sta(SSID,PASS);


            ESP_LOGI(TAG,"INIT wifi_init_esp_now");

        // ESP_ERROR_CHECK(esp_wifi_start());

            // ESP_ERROR_CHECK(esp_wifi_start());
        // wifi_init_sta(SSID,PASS);

    set_wifi_max_tx_power(MAX_RSSI);

    mqtt_init(BROKER, USER_NAME, NULL);
    subcribe_to_topic(TOPIC,2);
     xTaskCreate(mqtt_task, "mqtt_task", 5000, NULL, 5, NULL);
     xTaskCreate(esp_send_now, "send_to_esp_now", 3000, NULL, 5, NULL);
}
