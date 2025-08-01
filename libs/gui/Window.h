#pragma once

#include "Rect.h"
#include "Graphics.h"
#include <string>
#include <memory>

#include "View.h"

// Forward-declare the C window struct to avoid exposing kernel details
struct window_t;

namespace GUI {

class Window {
public:
    // Factory function to create a new window
    static std::unique_ptr<Window> create(const Rect& frame, const std::string& title);

    // Destructor
    ~Window();

    // Deleted copy and assignment operators to prevent slicing and enforce unique ownership
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // Window management
    void bring_to_front();
    void snap_left();
    void snap_right();

    // Drawing
    Graphics& graphics_context();
    void draw_rect(const Rect& rect, const Color& color);

    // Main drawing method to render the entire view hierarchy
    void draw();

    // Getters
    Rect frame() const;
    std::string title() const;

    // Access the root view of the window to add subviews
    std::shared_ptr<View> root_view() const;

private:
    // Private constructor, enforced by the factory method
    Window(window_t* handle);

    window_t* m_handle; // Pointer to the underlying C window struct
    Graphics& m_graphics_context; // Reference to the global graphics context
    std::shared_ptr<View> m_root_view; // Root view of the window
};

} // namespace GUI
