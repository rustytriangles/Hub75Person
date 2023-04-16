#include <algorithm>
#include <cstdio>
#include "person_sensor.hpp"

#include "hub75.hpp"

const uint32_t FB_WIDTH = 256;
const uint8_t FB_HEIGHT = 64;

const uint32_t GRID_WIDTH = 128;
const uint8_t GRID_HEIGHT = 128;

std::pair<unsigned int,unsigned int> grid_to_framebuffer(uint32_t x,uint32_t y,
                                     uint32_t grid_width, uint32_t grid_height,
                                     unsigned int fb_width, unsigned int fb_height) {
    unsigned int tx = (unsigned int)x;
    unsigned int ty = (unsigned int)y;
    if (ty < fb_height) {
        return std::make_pair(tx,ty);
    } else {
        tx = fb_width - 1 - tx;
        ty = (unsigned int)grid_height - 1 - ty;
        return std::make_pair(tx,ty);
    }
}

Hub75 hub75(FB_WIDTH, FB_HEIGHT, nullptr, PANEL_GENERIC, true);

volatile bool flip = false;

int interp(int min, int max, float t) {
    float v = (float)min + t * (float)(max-min);
    return std::max(min,std::min(max,(int)v));
}

void hub75_flip () {
    flip = true; // TODO: rewrite to semaphore
}

void __isr dma_complete() {
    hub75.dma_complete();
}

int main() {

    stdio_init_all();

    set_sys_clock_khz(200000, false);

    hub75.start(dma_complete);

    person_sensor_init();

    person_sensor_results_t prev_results = {};

    Pixel background(  32, 32, 32);
    Pixel colors[PERSON_SENSOR_MAX_FACES_COUNT] = {
        Pixel(255,  0,  0),
        Pixel(  0,255,  0),
        Pixel(  0,  0,255),
        Pixel(255,255,  0),
    };

    while (true) {
        person_sensor_results_t results = {};
        if (!person_sensor_read(&results)) {
            printf("No person sensor results found on the i2c bus\n");

        } else if (results != prev_results) {

            hub75.clear();


            if (results.num_faces == 0) {
                printf("I see no faces\n");
            } else {
                printf("I see %d faces\n", results.num_faces);
                for (int i=0; i<results.num_faces; i++) {
                    // printf("face[%d] = %d,%d,%d,%d\n",
                    //        i,
                    //        (int)results.faces[i].box_right,(int)results.faces[i].box_left,
                    //        (int)results.faces[i].box_top,(int)results.faces[i].box_bottom);
                    float left = 1.f - (float)results.faces[i].box_right / 255.f;
                    float right = 1.f - (float)results.faces[i].box_left / 255.f;
                    float top = (float)results.faces[i].box_top / 255.f;
                    float bottom = (float)results.faces[i].box_bottom / 255.f;
                    printf("left = %f, right = %f, bottom = %f, top = %f\n",left,right,bottom,top);
                    int xmin = interp(0, GRID_WIDTH-1, left);
                    int xmax = interp(0, GRID_WIDTH-1, right);
                    int ymin = interp(0, GRID_HEIGHT-1, top);
                    int ymax = interp(0, GRID_HEIGHT-1, bottom);

                    for (int y = ymin; y <= ymax; y++) {
                        for (int x = xmin; x<= xmax; x++) {
                            std::pair<uint,uint> fb_index = grid_to_framebuffer(x,y,GRID_WIDTH,GRID_HEIGHT,FB_WIDTH,FB_HEIGHT);
                            hub75.set_color(fb_index.first, fb_index.second, colors[i]);
                        }
                    }

                    // Debugging the case where the Y coord gets weird at the bottom
                    // for (int x = xmin; x<= xmax; x++) {
                    //     int y = ymin;
                    //     std::pair<uint,uint> fb_index = grid_to_framebuffer(x,y,GRID_WIDTH,GRID_HEIGHT,FB_WIDTH,FB_HEIGHT);
                    //     if (fb_index.first >= 0 && fb_index.first < FB_WIDTH && fb_index.second >= 0 && fb_index.second < FB_HEIGHT) {
                    //         hub75.set_color(fb_index.first, fb_index.second, colors[i]);
                    //     } else {
                    //         printf("bad coord %d,%d\n",fb_index.first,fb_index.second);
                    //     }
                    // }
                    // for (int y = ymin; y <= ymax; y++) {
                    //     int x = xmin;
                    //     std::pair<uint,uint> fb_index = grid_to_framebuffer(x,y,GRID_WIDTH,GRID_HEIGHT,FB_WIDTH,FB_HEIGHT);
                    //     if (fb_index.first >= 0 && fb_index.first < FB_WIDTH && fb_index.second >= 0 && fb_index.second < FB_HEIGHT) {
                    //         hub75.set_color(fb_index.first, fb_index.second, colors[i]);
                    //     } else {
                    //         printf("bad coord %d,%d\n",fb_index.first,fb_index.second);
                    //     }
                    // }

                }
            }

            prev_results = results;
            hub75.flip(true);
        }
        sleep_ms(10);
    }
}

