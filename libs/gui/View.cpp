#include "View.h"
#include <algorithm> // For std::find_if

namespace GUI {

View::View(const Rect& frame) : m_frame(frame) {}

void View::add_subview(std::shared_ptr<View> subview) {
    if (subview) {
        subview->m_superview = this;
        m_subviews.push_back(subview);
    }
}

void View::remove_from_superview() {
    if (m_superview) {
        auto& siblings = m_superview->m_subviews;
        siblings.erase(
            std::remove_if(siblings.begin(), siblings.end(),
                [this](const std::unique_ptr<View>& v) { return v.get() == this; }),
            siblings.end());
    }
}

void View::draw(Graphics& gfx) {
    // Base view does nothing, but we can draw subviews recursively
    for (const auto& subview : m_subviews) {
        // In a real implementation, we'd clip to the parent's bounds
        subview->draw(gfx);
    }
}

void View::on_mouse_down(MouseEvent& event) {
    // Default implementation passes the event to the superview.
    if (m_superview) {
        m_superview->on_mouse_down(event);
    }
}

void View::on_mouse_up(MouseEvent& event) {
    if (m_superview) {
        m_superview->on_mouse_up(event);
    }
}

void View::on_mouse_move(MouseEvent& event) {
    if (m_superview) {
        m_superview->on_mouse_move(event);
    }
}

} // namespace GUI
