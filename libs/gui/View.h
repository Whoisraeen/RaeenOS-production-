#pragma once

#include "Rect.h"
#include "Graphics.h"
#include "Event.h"
#include <vector>
#include <memory>
#include <string>

namespace GUI {

class View {
public:
    View(const Rect& frame = Rect());
    virtual ~View() = default;

    // Disallow copy and assign
    View(const View&) = delete;
    View& operator=(const View&) = delete;

    // Hierarchy management
    void add_subview(std::shared_ptr<View> subview);
    void remove_from_superview();

    // Drawing
    virtual void draw(Graphics& gfx);

    // Event Handling
    virtual void on_mouse_down(MouseEvent event) {};
    virtual void on_mouse_up(MouseEvent event) {};
    virtual void on_mouse_move(MouseEvent& event);

    // Layout
    Rect frame() const { return m_frame; }
    void set_frame(const Rect& frame) { m_frame = frame; }

    // Getters
    View* superview() const { return m_superview; }
    const std::vector<std::shared_ptr<View>>& subviews() const { return m_subviews; }

protected:
    Rect m_frame;
    View* m_superview = nullptr;
    std::vector<std::shared_ptr<View>> m_subviews;
};

} // namespace GUI
