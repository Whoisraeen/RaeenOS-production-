#pragma once

#include "View.h"
#include "Label.h"
#include "Color.h"
#include <string>
#include <functional>
#include <memory>

namespace GUI {

class Button : public View {
public:
    // Type for the click event handler
    using OnClickHandler = std::function<void()>;

    Button(const Rect& frame, const std::string& title);
    virtual ~Button() = default;

    // Public methods
    const std::string& title() const;
    void set_title(const std::string& title);

    void set_on_click(OnClickHandler handler);

    // Override View methods
    virtual void draw(Graphics& gfx) override;
    virtual void on_mouse_down(MouseEvent event) override;
    virtual void on_mouse_up(MouseEvent event) override;

private:
    std::shared_ptr<Label> m_label;
    OnClickHandler m_on_click_handler;

    // Visual states
    bool m_is_pressed = false;

    // Basic styling
    Color m_background_color = Color::light_gray();
    Color m_pressed_color = Color::dark_gray();
    Color m_border_color = Color::gray();
};

} // namespace GUI
