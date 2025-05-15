#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"

#include <stdio.h>
#include <string.h>

#include "lwip/sockets.h"
#include "lwip/inet.h"

#include "secrets.h"


uint16_t read_adc();

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

    adc_init();               // Initialize ADC peripheral
    adc_gpio_init(26);        // Enable ADC on GPIO 26
    adc_gpio_init(27);        // Enable ADC on GPIO 27


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




uint16_t read_adc()
{
    static uint input = 0;
    adc_select_input(input); // 0 for GPIO 26, 1 for GPIO 27
    uint16_t raw = adc_read();
    float voltage = raw * 3.3f / 4095;
    printf("ADC%u raw: %u, voltage: %.2f V\n", input, raw, voltage);
    input ^= 1; // Toggle between ADC 0 and 1
    return raw;
}
