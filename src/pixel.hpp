#include <iostream>
#include <tuple>

#pragma once

// This gamma table is used to correct our 8-bit (0-255) colours up to 11-bit,
// allowing us to gamma correct without losing dynamic range.
const uint16_t GAMMA_12BIT[256] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 47, 50,
    52, 54, 57, 59, 62, 65, 67, 70, 73, 76, 79, 82, 85, 88, 91, 94,
    98, 101, 105, 108, 112, 115, 119, 123, 127, 131, 135, 139, 143, 147, 151, 155,
    160, 164, 169, 173, 178, 183, 187, 192, 197, 202, 207, 212, 217, 223, 228, 233,
    239, 244, 250, 255, 261, 267, 273, 279, 285, 291, 297, 303, 309, 316, 322, 328,
    335, 342, 348, 355, 362, 369, 376, 383, 390, 397, 404, 412, 419, 427, 434, 442,
    449, 457, 465, 473, 481, 489, 497, 505, 513, 522, 530, 539, 547, 556, 565, 573,
    582, 591, 600, 609, 618, 628, 637, 646, 656, 665, 675, 685, 694, 704, 714, 724,
    734, 744, 755, 765, 775, 786, 796, 807, 817, 828, 839, 850, 861, 872, 883, 894,
    905, 917, 928, 940, 951, 963, 975, 987, 998, 1010, 1022, 1035, 1047, 1059, 1071, 1084,
    1096, 1109, 1122, 1135, 1147, 1160, 1173, 1186, 1199, 1213, 1226, 1239, 1253, 1266, 1280, 1294,
    1308, 1321, 1335, 1349, 1364, 1378, 1392, 1406, 1421, 1435, 1450, 1465, 1479, 1494, 1509, 1524,
    1539, 1554, 1570, 1585, 1600, 1616, 1631, 1647, 1663, 1678, 1694, 1710, 1726, 1743, 1759, 1775,
    1791, 1808, 1824, 1841, 1858, 1875, 1891, 1908, 1925, 1943, 1960, 1977, 1994, 2012, 2029, 2047};

// We don't *need* to make Pixel a fancy struct with RGB values, but it helps.
#pragma pack(push, 1)
struct alignas(4) Pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t _;

    constexpr Pixel() : r(0), g(0), b(0), _(0) {
    }

    constexpr Pixel(uint8_t r, uint8_t g, uint8_t b) :
        r(r), g(g), b(b), _(0) {
    }

};
#pragma pack(pop)

// Basic functions to convert between Hue, Saturation and Value and RGB
Pixel hsv_to_rgb(float h, float s, float v);

struct HsvType {
    std::tuple<float,float,float> value;

    HsvType(float h, float s, float v) :
        value(std::make_tuple(h,s,v)) {
    }
};

HsvType rgb_to_hsv(Pixel p);

// The following nonsense is really just to make boost test happy
std::ostream &operator<<(std::ostream &str, Pixel const &p);

bool operator==(const Pixel &a, const Pixel &b);

std::ostream &operator<<(std::ostream &str, const HsvType& hsv);

bool operator==(const HsvType& a, const HsvType& b);
