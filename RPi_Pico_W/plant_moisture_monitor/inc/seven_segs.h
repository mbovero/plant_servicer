#ifndef SEVEN_SEGS_H
#define SEVEN_SEGS_H

#include <stdint.h>

extern const uint8_t SEGMENT_PINS[];
extern const uint8_t DIGIT_PINS[];
extern const uint8_t DIGIT_PATTERNS[];

void init_display();
void display_number(uint16_t number);

#endif