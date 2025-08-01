#include "Button.h"
#include "Graphics.h"

namespace GUI {

Button::Button(const Rect& frame, const std::string& title)
    : View(frame) {
    // Create and configure the label for the button's text
    m_label = std::make_shared<Label>(Rect(0, 0, frame.width(), frame.height()), title);
    m_label->set_text_color(Color::black());
    // A more advanced implementation would have text alignment options
    // For now, we'll manually center it as best we can.
    // This requires knowing font metrics, which we don't have yet.
    // So we'll just do a rough alignment.
    int text_width = title.length() * 8; // Estimate width
    int text_height = 16; // Estimate height
    int label_x = (frame.width() - text_width) / 2;
    int label_y = (frame.height() - text_height) / 2;
    m_label->set_frame(Rect(label_x, label_y, text_width, text_height));

    add_subview(m_label);
}

const std::string& Button::title() const {
    return m_label->text();
}

void Button::set_title(const std::string& title) {
    m_label->set_text(title);
    // Recalculate label frame for centering if needed
    int text_width = title.length() * 8;
    int text_height = 16;
    int label_x = (frame().width() - text_width) / 2;
    int label_y = (frame().height() - text_height) / 2;
    m_label->set_frame(Rect(label_x, label_y, text_width, text_height));
}

void Button::set_on_click(OnClickHandler handler) {
    m_on_click_handler = std::move(handler);
}

void Button::draw(Graphics& gfx) {
    // Determine background color based on state
    Color current_bg_color = m_is_pressed ? m_pressed_color : m_background_color;

    // Draw background
    gfx.fill_rect(frame(), current_bg_color);

    // Draw border
    gfx.draw_rect(frame(), m_border_color);

    // Call base class draw to render subviews (our label)
    View::draw(gfx);
}

void Button::on_mouse_down(MouseEvent event) {
    if (frame().contains(event.position)) {
        m_is_pressed = true;
        // In a real system, we'd call a method like `set_needs_display()`
        // to invalidate this view and trigger a redraw.
    }
}

void Button::on_mouse_up(MouseEvent event) {
    if (m_is_pressed) {
        m_is_pressed = false;
        // Trigger the click handler if the mouse is released inside the button
        if (frame().contains(event.position) && m_on_click_handler) {
            m_on_click_handler();
        }
        // Invalidate and redraw
    }
}

} // namespace GUI
