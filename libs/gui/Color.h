#pragma once

#include <cstdint>

namespace GUI {

struct Color {
    uint8_t b = 0;
    uint8_t g = 0;
    uint8_t r = 0;
    uint8_t a = 255; // Alpha

    Color() = default;
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : b(b), g(g), r(r), a(a) {}

    // Create from a 32-bit integer (0xAARRGGBB)
    explicit Color(uint32_t value) {
        a = (value >> 24) & 0xFF;
        r = (value >> 16) & 0xFF;
        g = (value >> 8) & 0xFF;
        b = value & 0xFF;
    }

    uint32_t to_uint32() const {
        return (a << 24) | (r << 16) | (g << 8) | b;
    }

    // Predefined colors
    static Color black() { return Color(0, 0, 0); }
    static Color white() { return Color(255, 255, 255); }
    static Color red() { return Color(255, 0, 0); }
    static Color green() { return Color(0, 255, 0); }
    static Color blue() { return Color(0, 0, 255); }
    static Color light_gray() { return Color(211, 211, 211); }
    static Color gray() { return Color(128, 128, 128); }
    static Color dark_gray() { return Color(169, 169, 169); }
    static Color transparent() { return Color(0, 0, 0, 0); }
};

} // namespace GUI
