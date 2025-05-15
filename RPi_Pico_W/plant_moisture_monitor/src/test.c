


// #include "pico/stdlib.h"
// #include "hardware/adc.h"
// #include "pico/cyw43_arch.h"
// #include <stdio.h>
// #include <string.h>
// #include "lwip/ip4_addr.h"
// #include "lwip/sockets.h"
// #include "lwip/netdb.h"
// #include "lwip/inet.h"
// #include <errno.h>
// #include "secrets.h"

// int main() 
// {
//     stdio_init_all();         // Initialize USB serial

//     if (cyw43_arch_init()){
//         printf("Wi-fi init failed\n");
//         return -1;
//     }

//     // Prepare the the Wi-fi hardware to function as a client (so you can later connect to a specified SSID network)
//     cyw43_arch_enable_sta_mode();
//     printf("Connecting to Wi-fi...\n");

//     // Use the credentials from secrets.h
//     if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
//         printf("Failed to connect. \n");
//         return -1;
//     }
//     printf("Connected!\n");

//     // ADC setup
//     adc_init();               // Initialize ADC peripheral
//     adc_gpio_init(26);        // Enable ADC on GPIO 26
//     adc_gpio_init(27);        // Enable ADC on GPIO 27
//     bool adc_channel = 0; 

//     while (true){
//         adc_select_input(adc_channel);               // Select ADC0/ADC1 (GPIO 26/27)
//         uint16_t raw = adc_read();                   // 12-bit result (0–4095)
//         float voltage = raw * 3.3f / (1 << 12);      // Convert to volts
//         printf("ADC%u raw: %u, voltage: %.2f V\n", adc_channel, raw, voltage);

//         char message[64];
//         snprintf(message, sizeof(message), "ADC%u raw: %u, voltage: %.2f V\n", adc_channel, raw, voltage);

//         // Create socket and send data
//         int sock = socket(AF_INET, SOCK_STREAM, 0);
//         if (sock >= 0) {
//             struct sockaddr_in server_addr;
//             server_addr.sin_family = AF_INET;
//             server_addr.sin_port = htons(SERVER_PORT);
//             server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

//             if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
//                 write(sock, message, strlen(message));
//                 char response[64] = {0};
//                 read(sock, response, sizeof(response) - 1);  // Command from server
//                 printf("Server Response: %s\n", response);
//                 // Handle command if any (e.g., toggle ADC channel, change delay, etc.)
//             }

//             close(sock);
//         }

//         adc_channel ^= 1;
//         sleep_ms(500);
//     }




//     // void read_adc();
//     // while (true){
//     //     read_adc();
//     // }
            

//     return 0;
// }


// // void read_adc()
// // {
// //     static bool adc_channel = 0;
// //     adc_select_input(adc_channel);               // Select ADC0/ADC1 (GPIO 26/27)
// //     uint16_t raw = adc_read();                   // 12-bit result (0–4095)
// //     float voltage = raw * 3.3f / (1 << 12);      // Convert to volts
// //     printf("ADC%u raw: %u, voltage: %.2f V\n", adc_channel, raw, voltage);
// //     sleep_ms(500);  // Read twice a second
// //     adc_channel ^= 1;   // Swap between ADC0/ADC1
// // }




