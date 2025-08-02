/**
 * RaeenUI Hello World Example
 * Demonstrates basic RaeenUI framework usage
 */

#include "../raeenui.h"
#include "../../gpu/graphics_pipeline.h"

// Event handlers
bool handle_button_click(RaeenUIView* view, RaeenUIEvent* event) {
    printf("Hello World button clicked!\n");
    
    // Animate button color change
    RaeenUIAnimation* anim = raeenui_animate_color(view, 
        raeenui_color_hex(0x34C759), 0.3f); // Green color
    if (anim) {
        anim->curve = RAEENUI_ANIM_EASE_OUT;
        raeenui_start_animation(anim);
    }
    
    return true;
}

bool handle_theme_toggle(RaeenUIView* view, RaeenUIEvent* event) {
    // Get UI context from view (simplified)
    printf("Theme toggle clicked!\n");
    return true;
}

int main(void) {
    printf("RaeenUI Hello World Example\n");
    
    // Initialize graphics pipeline
    GraphicsPipeline* graphics = graphics_pipeline_create();
    if (!graphics_pipeline_initialize(graphics)) {
        printf("Failed to initialize graphics pipeline\n");
        return 1;
    }
    
    // Create UI context
    RaeenUIContext* ui = raeenui_create_context(graphics);
    if (!ui) {
        printf("Failed to create UI context\n");
        return 1;
    }
    
    if (!raeenui_initialize(ui)) {
        printf("Failed to initialize RaeenUI\n");
        return 1;
    }
    
    // Create main window
    RaeenUIWindow* window = raeenui_create_window(ui, "RaeenUI Hello World", 
        raeenui_rect_make(100, 100, 800, 600));
    if (!window) {
        printf("Failed to create window\n");
        return 1;
    }
    
    // Create UI layout using declarative syntax
    RaeenUIView* main_container = RaeenUIVStack(24);
    raeenui_padding(main_container, 32);
    raeenui_background(main_container, raeenui_color_hex(0xF8F9FA));
    
    // Title
    RaeenUIView* title = RaeenUIText("Welcome to RaeenUI!");
    raeenui_font(title, "RaeenUI-Bold", 32);
    raeenui_foreground(title, raeenui_color_hex(0x1D1D1F));
    raeenui_alignment(title, RAEENUI_ALIGN_CENTER, RAEENUI_ALIGN_CENTER);
    
    // Subtitle
    RaeenUIView* subtitle = RaeenUIText("Modern GPU-Accelerated UI Framework for RaeenOS");
    raeenui_font(subtitle, "RaeenUI-Regular", 18);
    raeenui_foreground(subtitle, raeenui_color_hex(0x6E6E73));
    raeenui_alignment(subtitle, RAEENUI_ALIGN_CENTER, RAEENUI_ALIGN_CENTER);
    
    // Feature list
    RaeenUIView* features_container = RaeenUIVStack(12);
    raeenui_padding(features_container, 24);
    raeenui_background(features_container, RAEENUI_COLOR_WHITE);
    raeenui_corner_radius(features_container, 12);
    raeenui_shadow(features_container, raeenui_color_rgba(0, 0, 0, 0.1f), 0, 4, 12);
    
    const char* features[] = {
        "✓ Declarative UI (SwiftUI/React-inspired)",
        "✓ GPU acceleration with Vulkan",
        "✓ Modern theming system",
        "✓ Smooth animations and effects",
        "✓ AI integration support",
        "✓ Accessibility built-in"
    };
    
    for (int i = 0; i < 6; i++) {
        RaeenUIView* feature = RaeenUIText(features[i]);
        raeenui_font(feature, "RaeenUI-Regular", 16);
        raeenui_foreground(feature, raeenui_color_hex(0x1D1D1F));
        raeenui_add_child_view(features_container, feature);
    }
    
    // Button container
    RaeenUIView* button_container = RaeenUIHStack(16);
    raeenui_alignment(button_container, RAEENUI_ALIGN_CENTER, RAEENUI_ALIGN_CENTER);
    
    // Main action button
    RaeenUIView* hello_button = RaeenUIButton("Say Hello!", handle_button_click);
    raeenui_background(hello_button, raeenui_color_hex(0x007AFF));
    raeenui_foreground(hello_button, RAEENUI_COLOR_WHITE);
    raeenui_corner_radius(hello_button, 12);
    raeenui_padding_edges(hello_button, 16, 32, 16, 32);
    raeenui_font(hello_button, "RaeenUI-Medium", 18);
    raeenui_shadow(hello_button, raeenui_color_rgba(0, 122, 255, 0.3f), 0, 4, 8);
    
    // Theme toggle button
    RaeenUIView* theme_button = RaeenUIButton("Toggle Theme", handle_theme_toggle);
    raeenui_background(theme_button, raeenui_color_hex(0x5856D6));
    raeenui_foreground(theme_button, RAEENUI_COLOR_WHITE);
    raeenui_corner_radius(theme_button, 12);
    raeenui_padding_edges(theme_button, 16, 32, 16, 32);
    raeenui_font(theme_button, "RaeenUI-Medium", 18);
    
    // AI Chat demo
    RaeenUIView* ai_chat = raeenui_create_ai_chat();
    raeenui_frame(ai_chat, 400, 200);
    raeenui_ai_enabled(ai_chat, "demo_assistant");
    
    RaeenUIView* ai_title = RaeenUIText("AI Assistant Integration");
    raeenui_font(ai_title, "RaeenUI-Medium", 16);
    raeenui_foreground(ai_title, raeenui_color_hex(0x5856D6));
    raeenui_add_child_view(ai_chat, ai_title);
    
    RaeenUIView* ai_text = RaeenUIText("AI-powered components are built into RaeenUI!");
    raeenui_font(ai_text, "RaeenUI-Regular", 14);
    raeenui_foreground(ai_text, raeenui_color_hex(0x3C3C43));
    raeenui_add_child_view(ai_chat, ai_text);
    
    // Build the UI hierarchy
    raeenui_add_child_view(button_container, hello_button);
    raeenui_add_child_view(button_container, theme_button);
    
    raeenui_add_child_view(main_container, title);
    raeenui_add_child_view(main_container, subtitle);
    raeenui_add_child_view(main_container, raeenui_create_spacer());
    raeenui_add_child_view(main_container, features_container);
    raeenui_add_child_view(main_container, raeenui_create_spacer());
    raeenui_add_child_view(main_container, button_container);
    raeenui_add_child_view(main_container, raeenui_create_spacer());
    raeenui_add_child_view(main_container, ai_chat);
    
    // Set as window's root view
    raeenui_add_child_view(window->root_view, main_container);
    
    // Apply layout
    raeenui_layout_window(window);
    
    // Show window
    raeenui_show_window(window);
    
    printf("RaeenUI Hello World window created and displayed\n");
    printf("Features demonstrated:\n");
    printf("- Declarative UI syntax\n");
    printf("- Modern styling and theming\n");
    printf("- Event handling\n");
    printf("- Animations\n");
    printf("- AI integration\n");
    printf("- GPU-accelerated rendering\n");
    
    // Main event loop (simplified)
    printf("Press Ctrl+C to exit\n");
    while (1) {
        // Render frame
        raeenui_render_frame(ui);
        
        // Handle events (simplified)
        // In a real implementation, this would integrate with the window manager
        
        // Sleep to limit frame rate
        // time_sleep_ms(16); // ~60 FPS
    }
    
    // Cleanup
    raeenui_destroy_window(window);
    raeenui_shutdown(ui);
    raeenui_destroy_context(ui);
    graphics_pipeline_destroy(graphics);
    
    return 0;
}
