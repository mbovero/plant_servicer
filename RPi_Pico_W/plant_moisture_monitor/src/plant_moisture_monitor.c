#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "lwip/tcp.h"
#include "secrets.h"
#include "seven_segs.h"

// Structure to hold our request data
typedef struct {
    char request[256];
    uint16_t length;
} http_request_t;

// WiFi connection settings
#define MAX_CONNECTION_ATTEMPTS 5
#define RETRY_DELAY_MS 2000

const uint BTN_PIN = 12;
bool btn_toggle = 0;
#define DEBOUNCE_MASK 0xF0000000  // Adjust mask for sensitivity

const float soil_density = 0.321;   // g/mL
const float pot_volume = 0.321;     // mL

uint16_t read_adc() {
    static uint input = 0;

    input = btn_toggle;

    adc_select_input(input);
    uint16_t raw = adc_read();
    float voltage = raw * 3.3f / 4095;
    printf("ADC%u raw: %u, voltage: %.2f V\n", input, raw, voltage);
    //input ^= 1;
    return raw;
}

static err_t tcp_sent_callback(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    printf("Data sent successfully\n");
    tcp_close(tpcb);
    return ERR_OK;
}

static err_t tcp_connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err) {
    if (err != ERR_OK) {
        printf("TCP connect failed\n");
        return err;
    }

    http_request_t *req = (http_request_t *)arg;
    
    printf("Sending %d bytes:\n", req->length);
    for(int i = 0; i < req->length; i++) {
        printf("%02x ", req->request[i]);
    }
    printf("\n");

    tcp_sent(tpcb, tcp_sent_callback);
    err_t write_err = tcp_write(tpcb, req->request, req->length, TCP_WRITE_FLAG_COPY);
    if (write_err != ERR_OK) {
        printf("Write error: %d\n", write_err);
        tcp_close(tpcb);
        return write_err;
    }
    
    err_t output_err = tcp_output(tpcb);
    if (output_err != ERR_OK) {
        printf("Output error: %d\n", output_err);
        tcp_close(tpcb);
        return output_err;
    }
    
    return ERR_OK;
}

void send_adc_to_server(uint16_t adc_value) {
    static http_request_t req;
    
    char json_body[32];
    int body_len = snprintf(json_body, sizeof(json_body), "{\"value\":%d}", adc_value);
    
    req.length = snprintf(req.request, sizeof(req.request),
        "POST /update HTTP/1.1\r\n"
        "Host: %s:%d\r\n"
        "User-Agent: PicoW\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        SERVER_IP, SERVER_PORT, body_len, json_body);

    printf("Constructed request (%d bytes):\n%.*s\n", req.length, req.length, req.request);

    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Failed to create PCB\n");
        return;
    }

    ip_addr_t addr;
    if (!ipaddr_aton(SERVER_IP, &addr)) {
        printf("Invalid IP\n");
        tcp_close(pcb);
        return;
    }

    err_t err = tcp_connect(pcb, &addr, SERVER_PORT, tcp_connected_callback);
    if (err != ERR_OK) {
        printf("TCP connect error: %d\n", err);
        tcp_close(pcb);
        return;
    }

    tcp_arg(pcb, &req);
}

bool connect_to_wifi() {
    printf("Initializing WiFi...\n");
    if (cyw43_arch_init()) {
        printf("WiFi init failed\n");
        return false;
    }

    cyw43_arch_enable_sta_mode();
    
    for (int attempt = 1; attempt <= MAX_CONNECTION_ATTEMPTS; attempt++) {
        printf("Connection attempt %d/%d...\n", attempt, MAX_CONNECTION_ATTEMPTS);
        
        int result = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, 
                                                      CYW43_AUTH_WPA2_AES_PSK, 30000);
        if (result == 0) {
            printf("Connected successfully!\n");
            return true;
        }
        
        printf("Connection failed (attempt %d), retrying in %dms...\n", 
               attempt, RETRY_DELAY_MS);
        
        // Blink LED to indicate retry
        for (int i = 0; i < 2; i++) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            sleep_ms(100);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            sleep_ms(100);
        }
        
        sleep_ms(RETRY_DELAY_MS);
    }
    
    printf("Max connection attempts reached\n");
    return false;
}

int main() {
    stdio_init_all();

    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
    
    init_display();
    gpio_init(BTN_PIN);
    gpio_set_dir(BTN_PIN, GPIO_IN);

    // // Initialize WiFi with retries
    // if (!connect_to_wifi()) {
    //     printf("Failed to connect to WiFi after multiple attempts\n");
    //     return 1;
    // }
    
    // // Blink LED quickly to indicate successful connection
    // for (int i = 0; i < 8; i++) {
    //     cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    //     sleep_ms(100);
    //     cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    //     sleep_ms(100);
    // }
    
    printf("Starting main loop...\n");
    
    while (true) {
        uint16_t adc_value = read_adc();
        double voltage = adc_value * 3.3f / 4095;
        double wsd_mL_ratio = 1.91/0.528; // This is a ratio between water-soil density and moisture sensor voltage [V / (g/mL)]
        double pot_wsd = wsd_mL_ratio * voltage; // Water-soil density currently in the pot

        double ws_mass = pot_wsd * pot_volume;
        double soil_mass = soil_density * pot_volume;
        double water_mass = (ws_mass - soil_mass);


        static uint32_t btn_debounce = 0;
        static bool btn_state = 0;
        static bool prev_btn_state = 0;

        prev_btn_state = btn_state;
        // Shift in current state (1 if pressed, 0 if released)
        btn_debounce = (btn_debounce << 1) | (gpio_get(BTN_PIN) ? 1 : 0);
        
        // Check for debounced press (all 1s in upper bits)
        if ((btn_debounce & DEBOUNCE_MASK) == DEBOUNCE_MASK) {
            btn_state = true;
        } 
        // Check for debounced release (all 0s in upper bits)
        else if ((btn_debounce & DEBOUNCE_MASK) == 0) {
            btn_state = false;
        }

        if (!prev_btn_state && btn_state)
            btn_toggle = !btn_toggle;

        // display_number(adc_value);
        //display_number(voltage*1000);
        display_number(ws_mass*1000);
        // send_adc_to_server(adc_value);
        // sleep_ms(5000);
    }
}