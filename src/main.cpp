#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_now.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_log.h"

static const char *TAG = "Slave";


uint8_t peer_mac[ESP_NOW_ETH_ALEN] = {0x3C, 0xE9, 0x0E, 0x87, 0x2F, 0xE0};


typedef struct struct_message {
    int temperature;
    int humidity;
    char message[32];
} struct_message_t;

static struct_message_t my_data;

void send_data() {
    
    my_data.temperature = rand() % 100;
    my_data.humidity = rand() % 100;
    strcpy(my_data.message, "Chamber 1 Slave");

   
    esp_err_t result = esp_now_send(peer_mac, (uint8_t *) &my_data, sizeof(my_data));

    if (result == ESP_OK) {
        ESP_LOGI(TAG, "Data sent successfully");
    } else {
        ESP_LOGE(TAG, "Error sending data: %d", result);
    }
}

void init_esp_now() {
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_set_pmk((uint8_t *) "pmk1234567890123"));

    // Register the peer
    esp_now_peer_info_t peer_info = {};
    memcpy(peer_info.peer_addr, peer_mac, ESP_NOW_ETH_ALEN);
    peer_info.channel = 0;
    peer_info.ifidx = WIFI_IF_STA;  
    peer_info.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&peer_info));
}

extern "C" void app_main() {
    
    ESP_ERROR_CHECK(nvs_flash_init());
    init_esp_now();
    while (true) {
        send_data();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
