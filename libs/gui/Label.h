#pragma once

#include "View.h"
#include "Color.h"
#include <string>

namespace GUI {

class Label : public View {
public:
    Label(const Rect& frame, std::string text);

    void draw(Graphics& gfx) override;

    const std::string& text() const { return m_text; }
    void set_text(const std::string& text) { m_text = text; }

    const Color& text_color() const { return m_text_color; }
    void set_text_color(const Color& color) { m_text_color = color; }

private:
    std::string m_text;
    Color m_text_color = Color::black();
};

} // namespace GUI
