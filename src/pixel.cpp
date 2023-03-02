#include <cmath>

#include "pixel.hpp"

std::ostream &operator<<(std::ostream &str, Pixel const &p) {
    str << "[" << (int)p.r << ", " << (int)p.g << ", " << (int)p.b << "]";
    return str;
}

bool operator==(const Pixel &a, const Pixel &b) {
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

Pixel hsv_to_rgb(float h, float s, float v) {
    if (h < 0.0f) {
        h = 1.0f + fmod(h, 1.0f);
    }

    int i = int(h * 6);
    float f = h * 6 - i;

    v = v * 255.0f;

    float sv = s * v;
    float fsv = f * sv;

    auto p = uint8_t(-sv + v);
    auto q = uint8_t(-fsv + v);
    auto t = uint8_t(fsv - sv + v);

    uint8_t bv = uint8_t(v);

    switch (i % 6) {
    default:
    case 0:
        return Pixel(bv, t, p);
    case 1:
        return Pixel(q, bv, p);
    case 2:
        return Pixel(p, bv, t);
    case 3:
        return Pixel(p, q, bv);
    case 4:
        return Pixel(t, p, bv);
    case 5:
        return Pixel(bv, p, q);
    }
}

HsvType rgb_to_hsv(Pixel p) {
    float r = (float)p.r / 255.f;
    float g = (float)p.g / 255.f;
    float b = (float)p.b / 255.f;

    auto cmax = std::max(r, std::max(g, b));
    auto cmin = std::min(r, std::min(g, b));
    auto diff = cmax - cmin;

    float h = 0.f;
    if (cmax == cmin) {
        h = 0.;
    } else if (cmax == r) {
        h = fmod(60. * ((g-b) / diff) + 360., 360.) / 360.;
    } else if (cmax == g) {
        h = fmod(60. * ((b-r) / diff) + 120., 360.) / 360.;
    } else if (cmax == b) {
        h = fmod(60. * ((r-g) / diff) + 240., 360.) / 360.;
    }

    float s = 0.f;
    if (cmax == 0.) {
        s = 0.;
    } else {
        s = (diff / cmax);
    }

    float v = 0.;
    v = cmax;

    return HsvType(h,s,v);
}

std::ostream &operator<<(std::ostream &str, const HsvType& hsv) {
    str << "[" << std::get<0>(hsv.value) << ", " << std::get<1>(hsv.value) << ", " << std::get<2>(hsv.value) << "]";
    return str;
}

bool operator==(const HsvType& a, const HsvType& b) {
    return a.value == b.value;
}
