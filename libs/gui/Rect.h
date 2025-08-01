#pragma once

#include "Point.h"
#include "Size.h"
#include <algorithm> // For std::max and std::min

namespace GUI {

struct Rect {
    Point origin;
    Size size;

    Rect() = default;
    Rect(int x, int y, int width, int height) : origin(x, y), size(width, height) {}
    Rect(const Point& origin, const Size& size) : origin(origin), size(size) {}

    int x() const { return origin.x; }
    int y() const { return origin.y; }
    int width() const { return size.width; }
    int height() const { return size.height; }

    int left() const { return origin.x; }
    int top() const { return origin.y; }
    int right() const { return origin.x + size.width; }
    int bottom() const { return origin.y + size.height; }

    bool contains(const Point& point) const {
        return point.x >= left() && point.x < right() &&
               point.y >= top() && point.y < bottom();
    }

    bool intersects(const Rect& other) const {
        return !(right() < other.left() ||
                 other.right() < left() ||
                 bottom() < other.top() ||
                 other.bottom() < top());
    }

    Rect intersection(const Rect& other) const {
        int new_x = std::max(x(), other.x());
        int new_y = std::max(y(), other.y());
        int new_width = std::min(right(), other.right()) - new_x;
        int new_height = std::min(bottom(), other.bottom()) - new_y;

        if (new_width <= 0 || new_height <= 0) {
            return Rect();
        }
        return Rect(new_x, new_y, new_width, new_height);
    }
};

} // namespace GUI
