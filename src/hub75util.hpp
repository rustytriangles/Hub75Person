#include "pixel.hpp"

#pragma once

void init_pins() {
    // Set up allllll the GPIO
    gpio_init(PIN_R0); gpio_set_function(PIN_R0, GPIO_FUNC_SIO); gpio_set_dir(PIN_R0, true);
    gpio_init(PIN_G0); gpio_set_function(PIN_G0, GPIO_FUNC_SIO); gpio_set_dir(PIN_G0, true);
    gpio_init(PIN_B0); gpio_set_function(PIN_B0, GPIO_FUNC_SIO); gpio_set_dir(PIN_B0, true);

    gpio_init(PIN_R1); gpio_set_function(PIN_R1, GPIO_FUNC_SIO); gpio_set_dir(PIN_R1, true);
    gpio_init(PIN_G1); gpio_set_function(PIN_G1, GPIO_FUNC_SIO); gpio_set_dir(PIN_G1, true);
    gpio_init(PIN_B1); gpio_set_function(PIN_B1, GPIO_FUNC_SIO); gpio_set_dir(PIN_B1, true);

    gpio_init(PIN_ROW_A); gpio_set_function(PIN_ROW_A, GPIO_FUNC_SIO); gpio_set_dir(PIN_ROW_A, true);
    gpio_init(PIN_ROW_B); gpio_set_function(PIN_ROW_B, GPIO_FUNC_SIO); gpio_set_dir(PIN_ROW_B, true);
    gpio_init(PIN_ROW_C); gpio_set_function(PIN_ROW_C, GPIO_FUNC_SIO); gpio_set_dir(PIN_ROW_C, true);
    gpio_init(PIN_ROW_D); gpio_set_function(PIN_ROW_D, GPIO_FUNC_SIO); gpio_set_dir(PIN_ROW_D, true);
    gpio_init(PIN_ROW_E); gpio_set_function(PIN_ROW_E, GPIO_FUNC_SIO); gpio_set_dir(PIN_ROW_E, true);

    gpio_init(PIN_CLK); gpio_set_function(PIN_CLK, GPIO_FUNC_SIO); gpio_set_dir(PIN_CLK, true);
    gpio_init(PIN_STB); gpio_set_function(PIN_STB, GPIO_FUNC_SIO); gpio_set_dir(PIN_STB, true);
    gpio_init(PIN_OE); gpio_set_function(PIN_OE, GPIO_FUNC_SIO); gpio_set_dir(PIN_OE, true);
}

void push_buffer(const Pixel* buffer, int width, int height) {
    // Step through 0b00000001, 0b00000010, 0b00000100 etc
    for(auto bit = 1u; bit < 1 << 11; bit <<= 1) {
        // Since the display is in split into two equal halves, we step through y from 0 to HEIGHT / 2
        for(auto y = 0u; y < height / 2; y++) {

            // 1. Shift out pixel data
            //    Shift out width pixels to the top and bottom half of the display
            for(auto x = 0u; x < width; x++) {
                // Get the current pixel for top/bottom half
                // This is easy since we just need the pixels at X/Y and X/Y+HEIGHT/2
                Pixel pixel_top     = buffer[x*height + y];
                Pixel pixel_bottom  = buffer[x*height + y + height / 2];

                // Gamma correct the colour values from 8-bit to 11-bit
                uint16_t pixel_top_b  = GAMMA_12BIT[pixel_top.b];
                uint16_t pixel_top_g  = GAMMA_12BIT[pixel_top.g];
                uint16_t pixel_top_r  = GAMMA_12BIT[pixel_top.r];

                uint16_t pixel_bottom_b = GAMMA_12BIT[pixel_bottom.b];
                uint16_t pixel_bottom_g = GAMMA_12BIT[pixel_bottom.g];
                uint16_t pixel_bottom_r = GAMMA_12BIT[pixel_bottom.r];

                // Set the clock low while we set up the data pins
                gpio_put(PIN_CLK, !CLK_POLARITY);

                // Top half
                gpio_put(PIN_R0, (bool)(pixel_top_r & bit));
                gpio_put(PIN_G0, (bool)(pixel_top_g & bit));
                gpio_put(PIN_B0, (bool)(pixel_top_b & bit));

                // Bottom half
                gpio_put(PIN_R1, (bool)(pixel_bottom_r & bit));
                gpio_put(PIN_G1, (bool)(pixel_bottom_g & bit));
                gpio_put(PIN_B1, (bool)(pixel_bottom_b & bit));

                // Wiggle the clock
                // The gamma correction above will ensure our clock stays asserted
                // for some small amount of time, avoiding the need for an explicit delay.
                gpio_put(PIN_CLK, CLK_POLARITY);
            }


            // 2. Set address pins
            //    Set the address pins to reflect the row to light up: 0 through 15 for 32x32 pixel panels
            //    We decode our 5-bit row address out onto the 5 GPIO pins by masking each bit in turn.
            gpio_put_masked(0b11111 << PIN_ROW_A, y << PIN_ROW_A);

            // 3. Assert latch/strobe signal (STB)
            //    This latches all the values we've just clocked into the column shift registers.
            //    The values will appear on the output pins, ready for the display to be driven.
            gpio_put(PIN_STB, STB_POLARITY);
            asm volatile("nop \nnop"); // Batman!
            gpio_put(PIN_STB, !STB_POLARITY);

            // 4. Asset the output-enable signal (OE)
            //    This turns on the display for a brief period to light the selected rows/columns.
            gpio_put(PIN_OE, OE_POLARITY);

            // 5. Delay
            //    Delay for a period of time coressponding to "bit"'s significance
            for(auto s = 0u; s < bit; ++s) {
                // The basic premise here is that "bit" will step through the values:
                // 1, 2, 4, 8, 16, 32, 64, etc in sequence.
                // If we plug this number into a delay loop, we'll get different magnitudes
                // of delay which correspond exactly to the significance of each bit.
                // The longer we delay here, the slower the overall panel refresh rate will be.
                // But we need to delay *just enough* that we're not under-driving the panel and
                // losing out on brightness.
                asm volatile("nop \nnop"); // Batman!
            }

            // 6. De-assert output-enable signal (OE)
            //    Ready to go again!
            gpio_put(PIN_OE, !OE_POLARITY);

            // 7. GOTO 1.
        }
        sleep_us(1);
    }
}
