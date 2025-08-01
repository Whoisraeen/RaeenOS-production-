#include "Window.h"
#include "c_wrappers.h"
#include <stdexcept> // For std::runtime_error

namespace GUI {

std::unique_ptr<Window> Window::create(const Rect& frame, const std::string& title) {
    window_t* handle = gui_window_create(frame.x(), frame.y(), frame.width(), frame.height());
    if (!handle) {
        // In a real scenario, you might want a more specific exception
        throw std::runtime_error("Failed to create window handle.");
    }

    // The title is now set within the C wrapper, which handles allocation.
    gui_window_set_title(handle, title.c_str());

    return std::unique_ptr<Window>(new Window(handle));
}

Window::Window(window_t* handle)
    : m_handle(handle),
      m_graphics_context(Graphics::get()),
      m_root_view(std::make_shared<View>(frame())) {}

Window::~Window() {
    // A `gui_window_destroy(m_handle)` function should eventually be created
    // to properly release all associated kernel resources. For now, we do nothing
    // in the destructor to avoid improper resource management.
}

void Window::bring_to_front() {
    if (m_handle) {
        gui_window_bring_to_front(m_handle);
    }
}

void Window::snap_left() {
    if (m_handle) {
        gui_window_snap_left(m_handle);
    }
}

void Window::snap_right() {
    if (m_handle) {
        gui_window_snap_right(m_handle);
    }
}

Graphics& Window::graphics_context() {
    return m_graphics_context;
}

void Window::draw_rect(const Rect& rect, const Color& color) {
    gui_window_draw_rect_in_window(m_handle, rect.x(), rect.y(), rect.width(), rect.height(), color.to_uint32());
}

void Window::draw() {
    if (m_root_view) {
        // In a real scenario, we might clear the window's buffer first.
        // m_graphics_context.clear_screen(Color::white());

        m_root_view->draw(m_graphics_context);

        // And then swap buffers to display the result.
        // m_graphics_context.swap_buffers();
    }
}

std::shared_ptr<View> Window::root_view() const {
    return m_root_view;
}

Rect Window::frame() const {
    if (m_handle) {
        return Rect(gui_window_get_x(m_handle),
                    gui_window_get_y(m_handle),
                    gui_window_get_width(m_handle),
                    gui_window_get_height(m_handle));
    }
    return Rect();
}

std::string Window::title() const {
    if (m_handle) {
        return std::string(gui_window_get_title(m_handle));
    }
    return "";
}

} // namespace GUI
