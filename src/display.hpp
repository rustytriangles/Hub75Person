#include <cmath>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <stdexcept>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/vreg.h"

#pragma once

const uint32_t FB_WIDTH = 128;
const uint8_t FB_HEIGHT = 64;

// Settings below are correct for I76, change them to suit your setup:

// Top half of display - 16 rows on a 32x32 panel
const uint PIN_R0 = 0;
const uint PIN_G0 = 1;
const uint PIN_B0 = 2;

// Bottom half of display - 16 rows on a 64x64 panel
const uint PIN_R1 = 3;
const uint PIN_G1 = 4;
const uint PIN_B1 = 5;

// Address pins, 5 lines = 2^5 = 32 values (max 64x64 display)
const uint PIN_ROW_A = 6;
const uint PIN_ROW_B = 7;
const uint PIN_ROW_C = 8;
const uint PIN_ROW_D = 9;
const uint PIN_ROW_E = 10;

// Sundry things
const uint PIN_CLK = 11;    // Clock
const uint PIN_STB = 12;    // Strobe/Latch
const uint PIN_OE = 13;     // Output Enable

const bool CLK_POLARITY = 1;
const bool STB_POLARITY = 1;
const bool OE_POLARITY = 0;

// User buttons and status LED
const uint PIN_SW_A = 14;
const uint PIN_SW_USER = 23;

const uint PIN_LED_R = 16;
const uint PIN_LED_G = 17;
const uint PIN_LED_B = 18;

// Required for FM6126A-based displays which need some register config/init to work properly
void FM6126A_write_register(uint16_t value, uint8_t position) {
    uint8_t threshold = FB_WIDTH - position;
    for(auto i = 0u; i < FB_WIDTH; i++) {
        auto j = i % 16;
        bool b = value & (1 << j);
        gpio_put(PIN_R0, b);
        gpio_put(PIN_G0, b);
        gpio_put(PIN_B0, b);
        gpio_put(PIN_R1, b);
        gpio_put(PIN_G1, b);
        gpio_put(PIN_B1, b);

        // Assert strobe/latch if i > threshold
        // This somehow indicates to the FM6126A which register we want to write :|
        gpio_put(PIN_STB, i > threshold);
        gpio_put(PIN_CLK, CLK_POLARITY);
        sleep_us(10);
        gpio_put(PIN_CLK, !CLK_POLARITY);
    }
}
