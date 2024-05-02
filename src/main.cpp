#include "esp_now.h"
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_wifi.h"
#include "driver/gpio.h"

extern "C" {
    int something = 10;
    // Define the structure for shared variables
    uint8_t macAddress[6] = {0x3C, 0xE9, 0x0E, 0x87, 0x2F, 0xE0}; // receiver mac address
    struct variables {
        int data;

    }   variable;

    void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

    void espnowSend(void *Parameter) {
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        esp_wifi_init(&cfg);
        esp_wifi_set_mode(WIFI_MODE_STA);
        esp_wifi_set_channel(0, WIFI_SECOND_CHAN_NONE);
        esp_wifi_start();

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
        xTaskCreatePinnedToCore(&espnowSend, "EspNowTask", 10000, (void*)&variable, 1, NULL, 0);
        xTaskCreatePinnedToCore(&NullTask, "NullTask", 2048, (void*)&variable, 1, NULL, 1);
    }
}