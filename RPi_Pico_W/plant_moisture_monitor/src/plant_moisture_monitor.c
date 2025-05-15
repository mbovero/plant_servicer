#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

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

int main() 
{
    // Initialize stdio for serial output
    stdio_init_all();
    
    // Initialize ADC hardware
    adc_init();
    
    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(26);
    adc_gpio_init(27);
    
    printf("Pico W ADC Reader\n");
    
    while (1) 
    {
        // Read both ADC channels
        uint16_t result = read_adc();
        
        // Simple delay - adjust as needed
        sleep_ms(500);
    }
    
    return 0;
}