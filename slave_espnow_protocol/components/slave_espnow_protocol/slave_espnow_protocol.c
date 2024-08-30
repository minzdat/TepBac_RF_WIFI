#include "slave_espnow_protocol.h"

static bool sleep_mode;
static char payload[MAX_PAYLOAD_LEN]; 
static const uint8_t s_slave_broadcast_mac[ESP_NOW_ETH_ALEN] = SLAVE_BROADCAST_MAC;
static QueueHandle_t s_slave_espnow_queue;
static slave_espnow_send_param_t send_param;
static slave_espnow_send_param_t send_param_specified;
static uint16_t s_espnow_seq[ESPNOW_DATA_MAX] = { 0, 0 };
mac_master_t s_master_unicast_mac;

void prepare_payload(espnow_data_t *espnow_data, float temperature_mcu, int rssi, float temperature_rdo, float do_value, float temperature_phg, float ph_value, const char *message) 
{

    // Initialize variable sensor_data_t with input parameters
    sensor_data_t sensor_data = 
    {
        .temperature_mcu = temperature_mcu,
        .rssi = rssi,
        .temperature_rdo = temperature_rdo,
        .do_value = do_value,
        .temperature_phg = temperature_phg,
        .ph_value = ph_value
    };

    // Calculate the size of the message
    size_t message_size = strlen(message);

    // Copy message to sensor_data, ensuring not to exceed the allocated size
    strncpy(sensor_data.message, message, message_size);

    // Calculate the size of sensor_data_t
    size_t sensor_data_size = sizeof(sensor_data_t);

    // Make sure that the sensor_data_t size is not larger than the fixed size of the payload
    if (sensor_data_size > MAX_PAYLOAD_LEN) 
    {
        ESP_LOGE(TAG, "The sensor_data_t data is too large to fit in the payload");
        return;
    }

    // Copy sensor_data_t into the payload and fill in the rest as needed
    memcpy(espnow_data->payload, &sensor_data, sensor_data_size);

    // If the data is less than 120 bytes, fill the remainder with 0
    if (sensor_data_size < MAX_PAYLOAD_LEN) 
    {
        memset(espnow_data->payload + sensor_data_size, 0, MAX_PAYLOAD_LEN - sensor_data_size);
    }

    // Print payload size and data for testing
    ESP_LOGI(TAG, "     Payload size: %d bytes", MAX_PAYLOAD_LEN);
    ESP_LOGI(TAG, "         MCU Temperature: %.2f", sensor_data.temperature_mcu);
    ESP_LOGI(TAG, "         RSSI: %d", sensor_data.rssi);
    ESP_LOGI(TAG, "         RDO Temperature: %.2f", sensor_data.temperature_rdo);
    ESP_LOGI(TAG, "         DO Value: %.2f", sensor_data.do_value);
    ESP_LOGI(TAG, "         PHG Temperature: %.2f", sensor_data.temperature_phg);
    ESP_LOGI(TAG, "         PH Value: %.2f", sensor_data.ph_value);
    ESP_LOGI(TAG, "         Message: %s", sensor_data.message);

}

/* Parse ESPNOW data payload. */
void parse_payload(const espnow_data_t *espnow_data) 
{
    if (sizeof(espnow_data->payload) < sizeof(sensor_data_t)) 
    {
        ESP_LOGE(TAG, "Payload size is too small to parse sensor_data_t");
        return;
    }

    sensor_data_t sensor_data;
    memcpy(&sensor_data, espnow_data->payload, sizeof(sensor_data_t));

    ESP_LOGI(TAG, "     Parsed ESPNOW payload:");
    ESP_LOGI(TAG, "         MCU Temperature: %.2f", sensor_data.temperature_mcu);
    ESP_LOGI(TAG, "         RSSI: %d", sensor_data.rssi);
    ESP_LOGI(TAG, "         RDO Temperature: %.2f", sensor_data.temperature_rdo);
    ESP_LOGI(TAG, "         DO Value: %.2f", sensor_data.do_value);
    ESP_LOGI(TAG, "         PHG Temperature: %.2f", sensor_data.temperature_phg);
    ESP_LOGI(TAG, "         PH Value: %.2f", sensor_data.ph_value);
    ESP_LOGI(TAG, "         Message: %s", sensor_data.message);

    memcpy(payload, sensor_data.message, strlen(sensor_data.message) + 1);
}

/* Prepare ESPNOW data to be sent. */
void espnow_data_prepare(slave_espnow_send_param_t *send_param, const char *message)
{
    espnow_data_t *buf = (espnow_data_t *)send_param->buffer;

    assert(send_param->len >= sizeof(espnow_data_t));

    buf->type = IS_BROADCAST_ADDR(send_param->dest_mac) ? ESPNOW_DATA_BROADCAST : ESPNOW_DATA_UNICAST;
    buf->seq_num = s_espnow_seq[buf->type]++;
    buf->crc = 0;

    // Log the data received
    ESP_LOGI(TAG, "Parsed ESPNOW packed:");
    ESP_LOGI(TAG, "     type: %d", buf->type);
    ESP_LOGI(TAG, "     seq_num: %d", buf->seq_num);
    ESP_LOGI(TAG, "     crc: %d", buf->crc);

    float temperature = read_internal_temperature_sensor();
    prepare_payload(buf, temperature, -45, 23.1, 7.6, 24.0, 7.2, message);

    buf->crc = esp_crc16_le(UINT16_MAX, (uint8_t const *)buf, send_param->len);
}

/* Parse received ESPNOW data. */
void espnow_data_parse(uint8_t *data, uint16_t data_len)
{
    espnow_data_t *buf = (espnow_data_t *)data;
    uint16_t crc, crc_cal = 0;

    if (data_len < sizeof(espnow_data_t)) 
    {
        ESP_LOGE(TAG, "Receive ESPNOW data too short, len:%d", data_len);
        return;
    }

    // Log the data received
    ESP_LOGI(TAG, "Parsed ESPNOW packed:");
    ESP_LOGI(TAG, "     type: %d", buf->type);
    ESP_LOGI(TAG, "     seq_num: %d", buf->seq_num);
    ESP_LOGI(TAG, "     crc: %d", buf->crc);

    // Log the payload if present
    if (data_len > sizeof(espnow_data_t)) 
    {
        parse_payload(buf);
    } 
    else 
    {
        ESP_LOGI(TAG, "  No payload data.");
    }

    crc = buf->crc;
    buf->crc = 0;
    crc_cal = esp_crc16_le(UINT16_MAX, (uint8_t const *)buf, data_len);

    if (crc_cal == crc) 
    {
        ESP_LOGI(TAG, "CRC check passed.");
    } 
    else 
    {
        ESP_LOGE(TAG, "CRC check failed. Calculated CRC: %d, Received CRC: %d", crc_cal, crc);
        return;
    }
}

void erase_peer(const uint8_t *peer_mac) 
{
    if (esp_now_is_peer_exist(peer_mac)) 
    {
        esp_err_t del_err = esp_now_del_peer(peer_mac);
        if (del_err != ESP_OK) 
        {
            ESP_LOGE(TAG, "Failed to delete peer " MACSTR ": %s",
                     MAC2STR(peer_mac), esp_err_to_name(del_err));
        } 
        else 
        {
            ESP_LOGI(TAG, "Deleted peer " MACSTR,
                     MAC2STR(peer_mac));
        }
    }
}

void add_peer(const uint8_t *peer_mac, bool encrypt) 
{   
    esp_now_peer_info_t peer; 

    memset(&peer, 0, sizeof(esp_now_peer_info_t));
    peer.channel = CONFIG_ESPNOW_CHANNEL;
    peer.ifidx = ESPNOW_WIFI_IF;
    peer.encrypt = encrypt;
    memcpy(peer.lmk, CONFIG_ESPNOW_LMK, ESP_NOW_KEY_LEN);
    memcpy(peer.peer_addr, peer_mac, ESP_NOW_ETH_ALEN);

    if (!esp_now_is_peer_exist(peer_mac)) 
    {
        ESP_ERROR_CHECK(esp_now_add_peer(&peer));
    } 
    else 
    {
        ESP_LOGI(TAG, "Peer already exists, modifying peer settings.");
        ESP_ERROR_CHECK(esp_now_mod_peer(&peer));
    }
}

/* Function to send a unicast response*/
esp_err_t response_specified_mac(const uint8_t *dest_mac, const char *message, bool encrypt)
{
    send_param_specified.len = MAX_DATA_LEN;

    memcpy(send_param_specified.dest_mac, dest_mac, ESP_NOW_ETH_ALEN);
    
    espnow_data_prepare(&send_param_specified, message);

    add_peer(dest_mac, encrypt);

    // Send the unicast response
    if (esp_now_send(send_param_specified.dest_mac, send_param_specified.buffer, send_param_specified.len) != ESP_OK) 
    {
        ESP_LOGE(TAG, "Send error");
        slave_espnow_deinit(&send_param_specified);
        vTaskDelete(NULL);
    }

    return ESP_OK;
}

void slave_espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    slave_espnow_event_t evt;
    slave_espnow_event_send_cb_t *send_cb = &evt.info.send_cb;

    if (mac_addr == NULL) 
    {
        ESP_LOGE(TAG, "Send cb arg error");
        return;
    }

    evt.id = SLAVE_ESPNOW_SEND_CB;
    memcpy(send_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    send_cb->status = status;
    // if (xQueueSend(s_slave_espnow_queue, &evt, ESPNOW_MAXDELAY) != pdTRUE) {
    //     ESP_LOGW(TAG, "Send send queue fail");
    // }
    ESP_LOGI(TAG, "Send callback: MAC Address " MACSTR ", Status: %s",
        MAC2STR(mac_addr), (status == ESP_NOW_SEND_SUCCESS) ? "Success" : "Fail");
}

void slave_espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    slave_espnow_event_t evt;
    slave_espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;
    uint8_t * mac_addr = recv_info->src_addr;
    uint8_t * des_addr = recv_info->des_addr;

    if (mac_addr == NULL || data == NULL || len <= 0) 
    {
        ESP_LOGE(TAG, "Receive cb arg error");
        return;
    }

    evt.id = SLAVE_ESPNOW_RECV_CB;
    memcpy(recv_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);

    if (len > MAX_DATA_LEN)
    {
        ESP_LOGE(TAG, "Received data length exceeds the maximum allowed");
        return;
    }

    recv_cb->data_len = len;
    memcpy(recv_cb->data, data, recv_cb->data_len);

    // if (xQueueSend(s_slave_espnow_queue, &evt, ESPNOW_MAXDELAY) != pdTRUE) {
    //     ESP_LOGW(TAG, "Send receive queue fail");
    //     free(recv_cb->data);
    // }

    // Check reception according to unicast 
    if (!IS_BROADCAST_ADDR(des_addr)) 
    {
        ESP_LOGI(TAG, "_________________________________");
        ESP_LOGI(TAG, "Receive unicast ESPNOW data");
        
        espnow_data_parse(recv_cb->data, recv_cb->data_len);

        switch (s_master_unicast_mac.connected) 
        {
            case false:
                // Check if the received data is MASTER_AGREE_CONNECT_MSG
                if (recv_cb->data_len >= strlen(MASTER_AGREE_CONNECT_MSG) && strstr((char *)payload, MASTER_AGREE_CONNECT_MSG) != NULL) 
                {
                    memcpy(s_master_unicast_mac.peer_addr, recv_cb->mac_addr, ESP_NOW_ETH_ALEN);
                    ESP_LOGI(TAG, "Added MAC Master " MACSTR " SUCCESS", MAC2STR(s_master_unicast_mac.peer_addr));
                    ESP_LOGW(TAG, "Response to MAC " MACSTR " SAVED MAC Master", MAC2STR(s_master_unicast_mac.peer_addr));
                    s_master_unicast_mac.connected = true;

                    save_to_nvs(NVS_KEY_CONNECTED, NVS_KEY_KEEP_CONNECT, s_master_unicast_mac.connected, s_master_unicast_mac.count_keep_connect);

                    s_master_unicast_mac.start_time =  esp_timer_get_time();
                    response_specified_mac(s_master_unicast_mac.peer_addr, SLAVE_SAVED_MAC_MSG, false);
                    add_peer(s_master_unicast_mac.peer_addr, true);
                }
                break;

            case true:
                // Check if the received data is CHECK_CONNECTION_MSG
                if (recv_cb->data_len >= strlen(CHECK_CONNECTION_MSG) && strstr((char *)payload, CHECK_CONNECTION_MSG) != NULL) 
                {
                    s_master_unicast_mac.start_time = esp_timer_get_time();
                    ESP_LOGW(TAG, "Response to MAC " MACSTR " %s", MAC2STR(s_master_unicast_mac.peer_addr),STILL_CONNECTED_MSG);
                    response_specified_mac(s_master_unicast_mac.peer_addr, STILL_CONNECTED_MSG, true);

                    s_master_unicast_mac.count_keep_connect = 0;

                    sleep_mode = true;
                }
                break;
        }
    }         
}

void slave_espnow_task(void *pvParameter)
{
    slave_espnow_send_param_t *send_param = (slave_espnow_send_param_t *)pvParameter;
    
    send_param->len = MAX_DATA_LEN;

    memcpy(send_param->dest_mac, s_slave_broadcast_mac, ESP_NOW_ETH_ALEN);

    while (true) 
    {
        switch (s_master_unicast_mac.connected) 
        {
            case false:

                erase_peer(s_master_unicast_mac.peer_addr);
           
                /* Start sending broadcast ESPNOW data. */
                ESP_LOGW(TAG, "---------------------------------");
                ESP_LOGW(TAG, "Start sending broadcast data");

                espnow_data_prepare(send_param, REQUEST_CONNECTION_MSG); 

                if (esp_now_send(send_param->dest_mac, send_param->buffer, send_param->len) != ESP_OK) 
                {
                    ESP_LOGE(TAG, "Send error");
                    slave_espnow_deinit(send_param);
                    vTaskDelete(NULL);
                }
                break;

            case true:

                s_master_unicast_mac.end_time =  esp_timer_get_time();
                uint64_t elapsed_time = s_master_unicast_mac.end_time - s_master_unicast_mac.start_time;

                ESP_LOGI(TAG, "Elapsed time: %llu microseconds", elapsed_time);

                if (elapsed_time > DISCONNECTED_TIMEOUT)
                {
                    // ESP_LOGW(TAG, "Time Out !");
                    
                    // s_master_unicast_mac.connected = false;

                    if (s_master_unicast_mac.count_keep_connect >= 3)
                    {
                        s_master_unicast_mac.connected = false;
                        s_master_unicast_mac.count_keep_connect = 0;
                        save_to_nvs(NVS_KEY_CONNECTED, NVS_KEY_KEEP_CONNECT, s_master_unicast_mac.connected, s_master_unicast_mac.count_keep_connect);
                        
                    }
                    
                    s_master_unicast_mac.count_keep_connect++;

                    save_to_nvs(NVS_KEY_CONNECTED, NVS_KEY_KEEP_CONNECT, s_master_unicast_mac.connected, s_master_unicast_mac.count_keep_connect);
                    
                    deep_sleep_mode();
                }


                if (sleep_mode)
                {
                    deep_sleep_mode();
                }
                
                break;
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1 seconds
    }
}

esp_err_t slave_espnow_init(void)
{
    s_slave_espnow_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(slave_espnow_event_t));
    if (s_slave_espnow_queue == NULL) 
    {
        ESP_LOGE(TAG, "Create mutex fail");
        return ESP_FAIL;
    }

    /* Initialize ESPNOW and register sending and receiving callback function. */
    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_send_cb(slave_espnow_send_cb) );
    ESP_ERROR_CHECK( esp_now_register_recv_cb(slave_espnow_recv_cb) );
// #if CONFIG_ESPNOW_ENABLE_POWER_SAVE
//     ESP_ERROR_CHECK( esp_now_set_wake_window(CONFIG_ESPNOW_WAKE_WINDOW) );
//     ESP_ERROR_CHECK( esp_wifi_connectionless_module_set_wake_interval(CONFIG_ESPNOW_WAKE_INTERVAL) );
// #endif
    /* Set primary slave key. */
    ESP_ERROR_CHECK( esp_now_set_pmk((uint8_t *)CONFIG_ESPNOW_PMK) );

    /* Add broadcast peer information to peer list. */
    add_peer(s_slave_broadcast_mac, false);

    /* Initialize sending parameters. */
    memset(&send_param, 0, sizeof(slave_espnow_send_param_t));

    return ESP_OK;
}

void slave_espnow_deinit(slave_espnow_send_param_t *send_param)
{
    vSemaphoreDelete(s_slave_espnow_queue);
    esp_now_deinit();
}

void slave_espnow_protocol()
{
    sleep_mode = false;
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    slave_wifi_init();

    init_temperature_sensor();

    //  ---Process values ​​from nvs---

    // erase_nvs(NVS_KEY_CONNECTED);
    load_from_nvs(NVS_KEY_CONNECTED, NVS_KEY_KEEP_CONNECT, &s_master_unicast_mac);
    ESP_LOGI("TAG", "s_master_unicast_mac.connected = %s", s_master_unicast_mac.connected ? "true" : "false");
    
    //  ---Process values ​​from nvs---

    deep_sleep_register_rtc_timer_wakeup();

    slave_espnow_init();

    xTaskCreate(slave_espnow_task, "slave_espnow_task", 4096, &send_param, 4, NULL);
}