#include "seven_segs.h"
#include "pico/stdlib.h"

// Define segment pins (a-g + dp)
const uint8_t SEGMENT_PINS[] = {0, 1, 2, 3, 4, 5, 6, 7}; // a,b,c,d,e,f,g,dp
const uint8_t DIGIT_PINS[] = {8, 9, 10, 11}; // Digit 1-4

// Segment patterns for digits 0-9
const uint8_t DIGIT_PATTERNS[] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111  // 9
};


void init_display() {
    // Initialize segment pins
    for (int i = 0; i < 8; i++) {
        gpio_init(SEGMENT_PINS[i]);
        gpio_set_dir(SEGMENT_PINS[i], GPIO_OUT);
        gpio_set_drive_strength(SEGMENT_PINS[i], GPIO_DRIVE_STRENGTH_8MA);
    }
    
    // Initialize digit pins
    for (int i = 0; i < 4; i++) {
        gpio_init(DIGIT_PINS[i]);
        gpio_set_dir(DIGIT_PINS[i], GPIO_OUT);
        gpio_set_drive_strength(DIGIT_PINS[i], GPIO_DRIVE_STRENGTH_12MA);
    }
}

void display_number(uint16_t number)
{
    uint8_t digits[4];
    
    // Extract individual digits
    digits[0] = number / 1000;
    digits[1] = (number / 100) % 10;
    digits[2] = (number / 10) % 10;
    digits[3] = number % 10;
    
    // Display each digit in sequence
    for (int d = 0; d < 4; d++) {
        // Turn off all digits
        for (int i = 0; i < 4; i++) {
            gpio_put(DIGIT_PINS[i], 0);
        }
        
        // Set segments for current digit
        for (int s = 0; s < 8; s++) {
            bool state = (DIGIT_PATTERNS[digits[d]] >> s) & 1;
            gpio_put(SEGMENT_PINS[s], !state);
        }
        
        // Turn on current digit
        gpio_put(DIGIT_PINS[d], 1); 
        
        // Short delay for visibility
        sleep_us(2000);
    }
}