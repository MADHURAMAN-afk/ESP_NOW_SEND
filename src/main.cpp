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


// // #include "esp_now.h"
// // #include <stdlib.h>
// // #include <string.h>
// // #include <freertos/FreeRTOS.h>
// // #include <freertos/task.h>
// // #include "esp_wifi.h"
// // #include "driver/gpio.h"

// // extern "C"{
// // // Define the structure for shared variables
// // struct variables {
// //     int data;
// // } variable;

// // uint8_t macAddress[6] = {0x3C, 0xE9, 0x0E, 0x87, 0x2F, 0xE0}; // receiver mac address

// // void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

// // void espnowSend(void *Parameter) {
// //     printf("MAIN CORE ENTERED\n\n");
// //     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
// //     esp_err_t ESP_WIFI_STATUS = esp_wifi_init(&cfg);
// //     printf("WIFI STATUS: %s", esp_err_to_name(ESP_WIFI_STATUS));

// //     esp_err_t ESP_WIFI_MODE_STATUS =esp_wifi_set_mode(WIFI_MODE_STA);
// //     printf("WIFI MODE Status: %s", esp_err_to_name(ESP_WIFI_MODE_STATUS));
// //     esp_err_t ESP_WIFI_CHANNEL_STATUS =esp_wifi_set_channel(0, WIFI_SECOND_CHAN_NONE);
// //     printf("WIFI CHANNEL STATUS: %s", esp_err_to_name(ESP_WIFI_CHANNEL_STATUS));
// //     esp_err_t ESP_WIFI_START_STATUS =esp_wifi_start();
// //     printf("WIFI START STATUS: %s", esp_err_to_name(ESP_WIFI_START_STATUS));
// //     esp_now_peer_info_t peerinfo;
// //     variables *myvar = (variables*)Parameter;

// //     // Initialize ESP-NOW
// //     esp_err_t esp_err = esp_now_init();
// //     if (esp_err != ESP_OK) {
// //         printf("Failed to initialize ESP-NOW: %s\n", esp_err_to_name(esp_err));
// //         vTaskDelete(NULL); // Terminate the task
// //     }
// //     else if (esp_err == ESP_OK) {
// //         printf("Passed to initialize ESP-NOW (nt ok):");
// //         vTaskDelete(NULL); // Terminate the task
// //     }

// //     // Register the send callback function
// //     esp_err = esp_now_register_send_cb(OnDataSent);
// //     if (esp_err != ESP_OK) {
// //         printf("Failed to register send callback: %s\n", esp_err_to_name(esp_err));
// //         vTaskDelete(NULL); // Terminate the task
// //     }

// //     // Add peer
// //     memcpy(peerinfo.peer_addr, macAddress, sizeof(macAddress));
// //     peerinfo.channel = 0;
// //     peerinfo.encrypt = false;
// //     // peerinfo.ifidx = WIFI_IF_STA;
// //     esp_err = esp_now_add_peer(&peerinfo);
// //     if (esp_err != ESP_OK) {
// //         printf("Failed to add peer: %s\n", esp_err_to_name(esp_err));
// //         vTaskDelete(NULL); // Terminate the task
// //     }

// //     while (1) {
// //         myvar->data = rand() % 1024;
// //         // Send data
// //         esp_err_t result = esp_now_send(macAddress, (uint8_t *)&myvar->data, sizeof(myvar->data));
// //         if (result == ESP_OK) {
// //             printf("Sending confirmed\n");
// //         } else {
// //             printf("Sending error %s\n", esp_err_to_name(result));
// //         }
// //         vTaskDelay(pdMS_TO_TICKS(5000));
// //     }
// // }

// // void OnDataSent(const uint8_t *macAddress, esp_now_send_status_t status) {
// //     printf("Data Sent Status: %s\n", status == ESP_NOW_SEND_SUCCESS ? "Success" : "Failed");
// // }

// // void NullTask(void *parameter) {
// //     gpio_config_t output = {
// //         .pin_bit_mask = (1ULL << GPIO_NUM_15),
// //         .mode = GPIO_MODE_OUTPUT,
// //         .pull_up_en = GPIO_PULLUP_DISABLE,
// //         .pull_down_en = GPIO_PULLDOWN_DISABLE,
// //         .intr_type = GPIO_INTR_DISABLE
// //     };
// //     gpio_config(&output);

// //     variables *myvar1 = (variables*)parameter;
// //     while (1) {
// //         printf("Hello from NullTask\n");
// //         printf("The data sent is %d from another core \n", myvar1->data);
// //         gpio_set_level(GPIO_NUM_15, 1); // Turn on the LED
// //         vTaskDelay(pdMS_TO_TICKS(500)); // Delay for 500 milliseconds
// //         gpio_set_level(GPIO_NUM_15, 0); // Turn off the LED
// //         vTaskDelay(pdMS_TO_TICKS(2000));
// //     }
// // }

// // void app_main() {
// //     // Create the ESP-NOW send task
// //     xTaskCreatePinnedToCore(&espnowSend, "EspNowTask", 10000, (void*)&variable, 1, NULL, 0);

// //     // Create the NullTask
// //     // xTaskCreatePinnedToCore(&NullTask, "NullTask", 2048, (void*)&variable, 1, NULL, 1);
// // }
// // }






// #include "esp_now.h"
// #include <stdlib.h>
// #include <string.h>
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include "esp_wifi.h"
// #include "driver/gpio.h"

// extern "C" {

// static const uint8_t receiver_mac[] = {0x3C, 0xE9, 0x0E, 0x87, 0x2F, 0xE0}; // Replace with the MAC address of the receiver

// static esp_now_peer_info_t peer;

// static void send_data(void *arg)
// {
//     const char *message = "Hello from the sender!";
//     esp_err_t result = esp_now_send(receiver_mac, (const uint8_t *)message, strlen(message));

//     if (result == ESP_OK) {
//         printf("Sent data: %s\n", message);
//     } else {
//         printf("Failed to send data\n");
//     }
// }

// static void init_espnow(void)
// {
//     ESP_ERROR_CHECK(esp_now_init());

//     memcpy(peer.peer_addr, receiver_mac, ESP_NOW_ETH_ALEN);
//     peer.channel = 0;
//     peer.encrypt = false;

//     esp_now_add_peer(&peer);
// }

// void app_main(void)
// { 
//     printf("Sender started\n");

//     // Initialize WiFi in STA mode
//     ESP_ERROR_CHECK(esp_wifi_init(NULL));
//     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

//     // Initialize ESP-NOW
//     init_espnow();

//     // Send data every 2 seconds
//     while (true) {
//         send_data(NULL);
//         vTaskDelay(pdMS_TO_TICKS(2000));
//     }
// }}