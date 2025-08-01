#include "Graphics.h"
#include "c_wrappers.h"

// We need to include the C headers here to call the kernel functions.
// The extern "C" block is crucial for linking C code with C++.
#include "c_wrappers.h"

namespace GUI {

Graphics& Graphics::get() {
    static Graphics instance;
    return instance;
}

int Graphics::init(multiboot_info_t* mboot_info) {
    return gui_graphics_init(mboot_info);
}

void Graphics::put_pixel(const Point& point, const Color& color) {
    gui_graphics_put_pixel(point.x, point.y, color.to_uint32());
}

void Graphics::draw_rect(const Rect& rect, const Color& color) {
    gui_graphics_draw_rect(rect.x(), rect.y(), rect.width(), rect.height(), color.to_uint32());
}

void Graphics::fill_rect(const Rect& rect, const Color& color) {
    gui_graphics_fill_rect(rect.x(), rect.y(), rect.width(), rect.height(), color.to_uint32());
}

void Graphics::clear_screen(const Color& color) {
    gui_graphics_clear_screen(color.to_uint32());
}

void Graphics::apply_blur(const Rect& rect, int radius) {
    // The C wrapper for blur wasn't added yet. This will be a no-op for now.
    // gui_graphics_apply_blur(rect.x(), rect.y(), rect.width(), rect.height(), radius);
}

void Graphics::swap_buffers() {
    gui_graphics_swap_buffers();
}

int Graphics::screen_width() const {
    return gui_graphics_get_width();
}

int Graphics::screen_height() const {
    return gui_graphics_get_height();
}

} // namespace GUI
