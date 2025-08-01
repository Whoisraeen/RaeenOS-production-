#pragma once

#include "Rect.h"
#include "Color.h"

// Forward-declare C types to keep kernel headers out of the public GUI API
struct multiboot_info_t;

namespace GUI {

// Graphics class provides a C++ wrapper around the kernel's C graphics API.
class Graphics {
public:
    // Singleton access
    static Graphics& get();

    // Deleted copy and assignment operators
    Graphics(const Graphics&) = delete;
    void operator=(const Graphics&) = delete;

    // Initialization
    int init(multiboot_info_t* mboot_info);

    // Drawing primitives
    void put_pixel(const Point& point, const Color& color);
    void draw_rect(const Rect& rect, const Color& color);
    void fill_rect(const Rect& rect, const Color& color);
    void clear_screen(const Color& color);

    // More advanced graphics operations
    void apply_blur(const Rect& rect, int radius);
    void swap_buffers();

    // Getters for screen dimensions
    int screen_width() const;
    int screen_height() const;

private:
    // Private constructor for singleton
    Graphics() = default;
};

} // namespace GUI
