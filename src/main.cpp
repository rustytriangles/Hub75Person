#include "display.hpp"
#include "hub75util.hpp"
#include "pixel.hpp"
#include "person_sensor.hpp"
#include <algorithm>

volatile bool flip = false;

Pixel backbuffer[FB_WIDTH][FB_HEIGHT];
Pixel frontbuffer[FB_WIDTH][FB_HEIGHT];

int interp(int min, int max, float t) {
    float v = (float)min + t * (float)(max-min);
    return std::max(min,std::min(max,(int)v));
}

void hub75_flip () {
    flip = true; // TODO: rewrite to semaphore
}

void clear_back_buffer(const Pixel& color) {
    for(auto x = 0u; x < FB_WIDTH; x++) {
        for(auto y = 0u; y < FB_HEIGHT; y++) {
            backbuffer[x][y] = color;
        }
    }
}

void clear_front_buffer(const Pixel& color) {
    for(auto x = 0u; x < FB_WIDTH; x++) {
        for(auto y = 0u; y < FB_HEIGHT; y++) {
            frontbuffer[x][y] = color;
        }
    }
}

void hub75_display_update() {

    // Ridiculous register write nonsense for the FM6126A-based 64x64 matrix
    FM6126A_write_register(0b1111111111111110, 12);
    FM6126A_write_register(0b0000001000000000, 13);

    while (true) {
        if (flip) {
            memcpy((uint8_t *)backbuffer, (uint8_t *)frontbuffer, FB_WIDTH * FB_HEIGHT * sizeof(Pixel));
            flip = false;
        }

        push_buffer(&backbuffer[0][0], FB_WIDTH, FB_HEIGHT);
    }
}

int main() {

    stdio_init_all();

    set_sys_clock_khz(200000, false);

    init_pins();

    multicore_launch_core1(hub75_display_update);

    person_sensor_init();

    person_sensor_results_t prev_results = {};

    Pixel background(  32, 32, 32);
    Pixel colors[PERSON_SENSOR_MAX_FACES_COUNT] = {
        Pixel(255,  0,  0),
        Pixel(  0,255,  0),
        Pixel(  0,  0,255),
        Pixel(255,255,  0),
    };

    clear_back_buffer(background);
    clear_front_buffer(background);
    while (true) {
        person_sensor_results_t results = {};
        if (!person_sensor_read(&results)) {
            printf("No person sensor results found on the i2c bus\n");

        } else if (results != prev_results) {

            clear_front_buffer(background);

            if (results.num_faces == 0) {
                printf("I see no faces\n");
            } else {
                printf("I see %d faces\n", results.num_faces);
                for (int i=0; i<results.num_faces; i++) {
                    float left = (float)results.faces[i].box_left / 255.f;
                    float right = (float)results.faces[i].box_right / 255.f;
                    float top = 1.f - (float)results.faces[i].box_top / 255.f;
                    float bottom = 1.f - (float)results.faces[i].box_bottom / 255.f;
                    for (int x = interp(0, FB_WIDTH, left);
                         x <= interp(0, FB_WIDTH, right);
                         x++) {
                        for (int y = interp(0,FB_HEIGHT-1,bottom);
                             y <= interp(0,FB_HEIGHT-1,top);
                             y++) {
                            frontbuffer[x][y] = colors[i];
                        }
                    }
                }

            }


            prev_results = results;
            hub75_flip();
        }
        sleep_ms(1);
    }
}

