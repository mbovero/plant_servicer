#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"

#include <stdio.h>
#include <string.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>
#include "secrets.h"

int main() 
{
    stdio_init_all();
    sleep_ms(2000); // Wait for USB serial to be ready

    if (cyw43_arch_init()) {
        printf("WiFi init failed!\n");
        return -1;
    }

    printf("Connecting to WiFi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Failed to connect to WiFi\n");
        return -1;
    }
    printf("Connected to WiFi!\n"); 

    stdio_init_all();         // Initialize USB serial
    adc_init();               // Initialize ADC peripheral
    adc_gpio_init(26);        // Enable ADC on GPIO 26
    adc_gpio_init(27);        // Enable ADC on GPIO 27

    void read_adc();


while (true) {
        uint16_t raw = adc_read();

        // Prepare JSON payload
        char json[64];
        snprintf(json, sizeof(json), "{ \"value\": %d }", raw);

        // Prepare HTTP request
        char request[512];
        snprintf(request, sizeof(request),
                 "POST /update HTTP/1.1\r\n"
                 "Host: %s\r\n"
                 "Content-Type: application/json\r\n"
                 "Content-Length: %zu\r\n\r\n"
                 "%s",
                 SERVER_IP, strlen(json), json);

        // Send request
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);
        server_addr.sin_addr.s_addr = ipaddr_addr(SERVER_IP);

        int sock = lwip_socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            printf("Socket error\n");
            continue;
        }

        if (lwip_connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
            printf("Connect failed\n");
            lwip_close(sock);
            continue;
        }

        lwip_write(sock, request, strlen(request));
        lwip_close(sock);

        printf("Sent: %s\n", json);
        sleep_ms(2000);
    }
}




void read_adc()
{
    static bool adc_channel = 0;
    adc_select_input(adc_channel);               // Select ADC0/ADC1 (GPIO 26/27)
    uint16_t raw = adc_read();                   // 12-bit result (0–4095)
    float voltage = raw * 3.3f / (1 << 12);      // Convert to volts
    printf("ADC%u raw: %u, voltage: %.2f V\n", adc_channel, raw, voltage);
    sleep_ms(500);  // Read twice a second
    adc_channel ^= 1;   // Swap between ADC0/ADC1
}



