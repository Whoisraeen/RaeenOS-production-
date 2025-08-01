#include "Window.h"
#include "Button.h"
#include <iostream>


// This is a conceptual main function demonstrating how the GUI framework
// is intended to be used. Since we are not compiling or running, this
// serves as a design document and usage example.

int main(int argc, char** argv) {
    // The application would start by initializing the GUI system.
    // This might involve setting up the graphics mode, etc.
    // For now, we assume this is handled by the Graphics singleton.

    // 1. Create a top-level window.
    // The `create` method returns a unique_ptr for ownership management.
    auto main_window = GUI::Window::create(GUI::Rect(100, 100, 400, 300), "My Application");

    // 2. Get the window's root view to add other UI elements to it.
    auto root_view = main_window->root_view();

    // 3. Create a Button.
    // The button is positioned relative to its parent's coordinate system (the root view).
    GUI::Rect button_frame(50, 50, 120, 30);
    auto my_button = std::make_shared<GUI::Button>(button_frame, "Click Me!");

    // 4. Set a handler for the button's click event.
    // We use a C++ lambda to define the action.
    my_button->set_on_click([]() {
        // In a real app, this could open a dialog, change data, etc.
        // For this example, we'll imagine it prints to the console.
        // std::cout << "Button was clicked!" << std::endl;
    });

    // 5. Add the button to the window's view hierarchy.
    root_view->add_subview(my_button);

    // 6. Enter the main event loop.
    // In a real application, this loop would:
    //   - Poll for events (mouse, keyboard, etc.) from the OS.
    //   - Dispatch events to the appropriate window and view.
    //   - Redraw the window's contents when necessary (e.g., after a click).
    //
    // while (application_is_running) {
    //     Event event = get_next_event();
    //     main_window->handle_event(event);
    //     main_window->draw();
    // }

    return 0;
}
