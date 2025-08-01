#include "Label.h"
#include "c_wrappers.h"

// Assume the kernel provides a C function for drawing text.
// We'll need to link against it.
extern "C" {
    // This function signature is an assumption based on the kernel's style.
    // It likely lives in a header like `font.h` or `graphics.h`.
    void graphics_draw_string(int x, int y, const char* str, uint32_t color);
}

namespace GUI {

Label::Label(const Rect& frame, std::string text)
    : View(frame),
      m_text(std::move(text)) {}

void Label::draw(Graphics& gfx) {
    // Call the base class draw to handle subviews, if any.
    View::draw(gfx);

    // Draw the label's text. We use the frame's origin for position.
    // A more advanced implementation would calculate the absolute position.
    gui_graphics_draw_string(frame().x(), frame().y(), m_text.c_str(), m_text_color.to_uint32());
}

} // namespace GUI
