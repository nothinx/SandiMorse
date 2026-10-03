// Arduino.h tiruan untuk menguji logika SandiMorse di PC.
#pragma once
#include <stddef.h>
#include <stdint.h>
#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define PROGMEM
#define pgm_read_byte(p) (*(const uint8_t *)(p))
extern uint32_t waktuPalsu;
extern int pinPalsu;
inline uint32_t millis() { return waktuPalsu; }
inline void pinMode(uint8_t, uint8_t) {}
inline void digitalWrite(uint8_t, uint8_t nilai) { pinPalsu = nilai; }
