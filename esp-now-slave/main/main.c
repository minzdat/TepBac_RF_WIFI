#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_now.h"

// MACRO for MAC address formatting
#define MAC2STR(a)             (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR                 "%02x:%02x:%02x:%02x:%02x:%02x"

// Tag for logging
static const char *TAG = "espnow_slave";
char messageq[] = "hi master___";

typedef struct {
    // int data;
        char data[250]; 
        char data2[250];
 // Chuỗi dữ liệu
} esp_now_message_t;
    esp_now_peer_info_t peer_info = {};

int count=0;
    bool addd=true;

// Address of the target device (for sending unicast)
//static uint8_t target_mac[ESP_NOW_ETH_ALEN] = { 0xf4, 0x12, 0xfa, 0x42, 0xa3, 0xdc };
//static uint8_t target_mac[6] = {0x48, 0x27, 0xe2, 0xc9, 0x7d, 0x08};
// Address of the sender device (for responding)
static uint8_t sender_mac[ESP_NOW_ETH_ALEN];
static uint8_t broadcast_mac[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
// Callback function to handle send status
static void example_espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    ESP_LOGI(TAG, "Send 1 callback: " MACSTR ", status: %d", MAC2STR(mac_addr), status);
}

// Callback function to handle received data
static void example_espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    ESP_LOGE(TAG, "Receive 1 callback: " MACSTR ", len: %d", MAC2STR(recv_info->src_addr), len);
    esp_now_message_t *message = (esp_now_message_t *)data;
    ESP_LOGI(TAG, "Received data: %s", message->data);
    count++;
    uint8_t response[50] ;
    sprintf((char *)response, "Response___: %d", count);
    ESP_LOGI(TAG,"%s",response);

    //ESP_LOGI(TAG, "Data: %.*s", len, data);
    //uint8_t * mac_addr = recv_info->src_addr;  ///note
    // Save sender MAC address
    memcpy(sender_mac, recv_info->src_addr, ESP_NOW_ETH_ALEN);


    if (addd){
        memcpy(peer_info.peer_addr, recv_info->src_addr, 6);
        esp_now_add_peer(&peer_info);
        addd=false;
    }
    // Send response to the sender device
    esp_err_t result = esp_now_send(recv_info->src_addr, response, sizeof(response));
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Send 1 error: %d", result);
    } else {
        ESP_LOGI(TAG, "Response sent to: " MACSTR, MAC2STR(sender_mac));
    }
}

static esp_err_t example_espnow_init(void)
{
    // Initialize ESP-NOW
    ESP_ERROR_CHECK(esp_now_init());

    // Register the send callback function
    // ESP_ERROR_CHECK(esp_now_register_send_cb(example_espnow_send_cb));

    // Register the receive callback function
    ESP_ERROR_CHECK(esp_now_register_recv_cb(example_espnow_recv_cb));
    ESP_ERROR_CHECK(esp_now_register_send_cb(example_espnow_send_cb));
    // Add a peer (target device) for unicast
    esp_now_peer_info_t peer_info = {
        .channel = 0, // Use the current channel
        .ifidx = ESP_IF_WIFI_STA,
        .encrypt = false,
    };
    
    memcpy(peer_info.peer_addr, sender_mac, ESP_NOW_ETH_ALEN);///
    ESP_ERROR_CHECK(esp_now_add_peer(&peer_info));

    // Add a broadcast peer (optional, as broadcast messages do not require peer addition)
    esp_now_peer_info_t broadcast_peer_info = {
        .channel = 0, // Use the current channel
        .ifidx = ESP_IF_WIFI_STA,
        .encrypt = false,
    };
    memset(broadcast_peer_info.peer_addr, 0xFF, ESP_NOW_ETH_ALEN); // Broadcast address
    ESP_ERROR_CHECK(esp_now_add_peer(&broadcast_peer_info));

    return ESP_OK;
}

static void example_espnow_task(void *pvParameter)
{
    const char *message = "Hello from Client!";
    size_t message_len = strlen(message);

    while (1) {
        // ESP_LOGI(TAG, "Sending message to " MACSTR ": %s", MAC2STR(target_mac), message);

        // Send data to the specified MAC address (unicast)
        // esp_err_t result = esp_now_send(target_mac, (const uint8_t *)message, message_len);
        // if (result != ESP_OK) {
        //     ESP_LOGE(TAG, "Send error: %d", result);
        // }

        // Delay before sending the next message
        vTaskDelay(2000 / portTICK_PERIOD_MS); // 2 seconds delay
    }
}

static void example_wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE));
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize WiFi and ESP-NOW
    example_wifi_init();
    ESP_ERROR_CHECK(example_espnow_init());

    // Create the task to send data
    xTaskCreate(example_espnow_task, "example_espnow_task", 2048, NULL, 4, NULL);
}
