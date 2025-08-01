#pragma once

namespace GUI {

struct Size {
    int width = 0;
    int height = 0;

    Size() = default;
    Size(int width, int height) : width(width), height(height) {}

    bool is_empty() const {
        return width <= 0 || height <= 0;
    }

    bool operator==(const Size& other) const {
        return width == other.width && height == other.height;
    }

    bool operator!=(const Size& other) const {
        return !(*this == other);
    }
};

} // namespace GUI
