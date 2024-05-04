#include "esp_now.h"
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_wifi.h"
#include "driver/gpio.h"
#include "nvs_flash.h"

extern "C" {
    int something = 10;
    // Define the structure for shared variables
    uint8_t macAddress[6] = {0x3C, 0xE9, 0x0E, 0x87, 0x2F, 0xE0}; // receiver mac address
    struct variables {
        int data;

    }   variable;

    void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

    void espnowSend(void *Parameter) {
        

        esp_now_peer_info_t peerinfo;
        variables *myvar = (variables*)Parameter;

        // Initialize ESP-NOW
        esp_err_t esp_err = esp_now_init();
        if (esp_err != ESP_OK) {
            printf("Failed to initialize ESP-NOW: %s\n", esp_err_to_name(esp_err));
            vTaskDelete(NULL); // Terminate the task
        }

        // Register the send callback function
        esp_err = esp_now_register_send_cb(OnDataSent);
        if (esp_err != ESP_OK) {
            printf("Failed to register send callback: %s\n", esp_err_to_name(esp_err));
            vTaskDelete(NULL); // Terminate the task
        }

        // Add peer
        memcpy(peerinfo.peer_addr, macAddress, sizeof(macAddress));
        peerinfo.channel = 0;
        peerinfo.encrypt = false;
        peerinfo.ifidx = WIFI_IF_STA;
        esp_err = esp_now_add_peer(&peerinfo);
        if (esp_err != ESP_OK) {
            printf("Failed to add peer: %s\n", esp_err_to_name(esp_err));
            vTaskDelete(NULL); // Terminate the task
        }

        while (1) {
            myvar->data = rand() % 1024;
            // Send data
            esp_err_t result = esp_now_send(macAddress, (uint8_t*) myvar, sizeof(myvar));
            if (result == ESP_OK) {
                printf("Sending confirmed\n");
            } else {
                printf("Sending error %s\n", esp_err_to_name(result));

                //   printf("Receiver MAC address: ");
                // for (int i = 0; i < 6; i++) {
                //     printf("%02X ", macAddress[i]);
                // }
                // printf("\n");
            }
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }

    void OnDataSent(const uint8_t *macAddress, esp_now_send_status_t status) {
        printf("Data Sent Status: %s\n", status == ESP_NOW_SEND_SUCCESS ? "Success" : "Failed");
    }

    void NullTask(void *parameter) {
        gpio_config_t output{
            .pin_bit_mask = 1ULL << GPIO_NUM_15,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        gpio_config(&output);

        variables *myvar1 = (variables*)parameter;
        while (1) {
            printf("Hello from blinky_2\n");
            printf("The data sent is %d from another core \n", myvar1->data);
            gpio_set_level(GPIO_NUM_15, 1); // Turn off the LED
            vTaskDelay(pdMS_TO_TICKS(500)); // Delay for 500 milliseconds
            gpio_set_level(GPIO_NUM_15, 0);
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }

    void app_main() {
        // uint8_t num = 10;
        // uint8_t *data;
        // data = &num;

        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_init());
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_loop_create_default());
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_storage(WIFI_STORAGE_RAM));
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_start());

        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK_WITHOUT_ABORT(ret);


        xTaskCreatePinnedToCore(&espnowSend, "EspNowTask", 10000, (void*)&variable, 1, NULL, 0);
        xTaskCreatePinnedToCore(&NullTask, "NullTask", 2048, (void*)&variable, 1, NULL, 1);
    }
}