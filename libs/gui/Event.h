#pragma once

#include "Point.h"

namespace GUI {

enum class MouseButton {
    Left,
    Right,
    Middle
};

struct MouseEvent {
    enum class Type {
        MouseDown,
        MouseUp,
        MouseMove,
        MouseEnter,
        MouseLeave
    };

    Type type;
    Point position;
    MouseButton button;
};

} // namespace GUI

#include "Point.h"

namespace GUI {

enum class MouseEventType {
    MouseDown,
    MouseUp,
    MouseMove,
    MouseEnter,
    MouseLeave
};

struct MouseEvent {
    MouseEventType type;
    Point position; // Position relative to the view
    // Add other relevant data like button type (left, right, etc.) later
};

} // namespace GUI
