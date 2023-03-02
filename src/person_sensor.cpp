#include "person_sensor.hpp"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include <stdio.h>

const uint I75_SDA_PIN = 20; // was PICO_DEFAULT_I2C_SDA_PIN
const uint I75_SCL_PIN = 21; // was PICO_DEFAULT_I2C_SCL_PIN

// Speed of the I2C bus. The sensor supports many rates but 400KHz works.
const int32_t I2C_BAUD_RATE = (400 * 1000);

void person_sensor_init() {
    // Use I2C0 on the default SDA and SCL pins (6, 7 on a Pico).
    i2c_init(i2c_default, I2C_BAUD_RATE);
    gpio_set_function(I75_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I75_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I75_SDA_PIN);
    gpio_pull_up(I75_SCL_PIN);
    // Make the I2C pins available to picotool.
    bi_decl(bi_2pins_with_func(I75_SDA_PIN,
                               I75_SCL_PIN, GPIO_FUNC_I2C));

    person_sensor_write_reg(PERSON_SENSOR_REG_MODE, PERSON_SENSOR_MODE_CONTINUOUS);
}

bool person_sensor_read(person_sensor_results_t *results) {
    int num_bytes_read = i2c_read_blocking(
        i2c_default,
        PERSON_SENSOR_I2C_ADDRESS,
        (uint8_t *)(results),
        sizeof(person_sensor_results_t),
        false);
    return (num_bytes_read == sizeof(person_sensor_results_t));
}

// Writes the value to the sensor register over the I2C bus.
void person_sensor_write_reg(uint8_t reg, uint8_t value) {
    uint8_t write_bytes[2] = {reg, value};
    i2c_write_blocking(
        i2c_default,
        PERSON_SENSOR_I2C_ADDRESS,
        write_bytes,
        2,
        false);
}

bool operator!=(const person_sensor_results_t& a,
                const person_sensor_results_t& b) {
    if (a.num_faces != b.num_faces) {
        return true;
    }
    for (int i=0; i<a.num_faces; i++) {
        if (a.faces[i].box_left != b.faces[i].box_left) {
            return true;
        }
        if (a.faces[i].box_top != b.faces[i].box_top) {
            return true;
        }
        if (a.faces[i].box_right != b.faces[i].box_right) {
            return true;
        }
        if (a.faces[i].box_bottom != b.faces[i].box_bottom) {
            return true;
        }
    }
    return false;
}

bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void scan_i2c_bus() {
    for (int addr = 0; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }

        // Perform a 1-byte dummy read from the probe address. If a peripheral
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr)) {
            ret = PICO_ERROR_GENERIC;
        }
        else {
            ret = i2c_read_blocking(i2c_default, addr, &rxdata, 1, false);
        }
        printf(ret < 0 ? "." : "@");
        printf(addr % 16 == 15 ? "\n" : "  ");
    }
    printf("Done.\n");
}
