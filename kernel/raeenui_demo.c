// RaeenUI Demo Implementation - Showcasing Revolutionary UI Concepts

#include "raeenui_vga.h"
#include "string.h"

// Demo state tracking
static struct {
    int current_demo;
    int demo_timer;
    bool ai_suggestions_enabled;
    int gaming_fps;
    int system_load;
    bool performance_overlay_visible;
} demo_state = {0};

// ============================================================================
// DEMO EVENT HANDLERS
// ============================================================================

bool demo_button_handler(ui_component_t* component, ui_event_t* event) {
    if (event->type == UI_EVENT_MOUSE_CLICK && event->mouse_left) {
        // Button click feedback
        if (component->text) {
            if (strcmp(component->text, "Next Demo") == 0) {
                demo_state.current_demo = (demo_state.current_demo + 1) % 4;
                return true;
            } else if (strcmp(component->text, "AI Assist") == 0) {
                demo_state.ai_suggestions_enabled = !demo_state.ai_suggestions_enabled;
                ui_set_text(component, demo_state.ai_suggestions_enabled ? "AI: ON" : "AI: OFF");
                return true;
            } else if (strcmp(component->text, "Performance") == 0) {
                demo_state.performance_overlay_visible = !demo_state.performance_overlay_visible;
                return true;
            }
        }
    }
    return false;
}

bool demo_window_close_handler(ui_component_t* component, ui_event_t* event) {
    if (event->type == UI_EVENT_MOUSE_CLICK && event->mouse_left) {
        // Simple window close - hide the window
        ui_set_visible(component, false);
        return true;
    }
    return false;
}

// ============================================================================
// DESKTOP ENVIRONMENT DEMO
// ============================================================================

void ui_demo_desktop_environment(ui_context_t* ctx) {
    // Clear screen with desktop background
    vga_clear_with_color(VGA_COLOR_BLUE);
    
    // Create taskbar
    ui_component_t* taskbar = ui_create_panel(ctx, 0, 22, 80, 3);
    ui_set_colors(taskbar, VGA_COLOR_WHITE, VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    
    // Start menu button
    ui_component_t* start_btn = ui_create_button(ctx, 1, 23, 12, "RaeenStart");
    ui_set_colors(start_btn, VGA_COLOR_WHITE, VGA_COLOR_GREEN, VGA_COLOR_DARK_GREY);
    ui_set_event_handler(start_btn, demo_button_handler);
    
    // System clock (fake)
    ui_component_t* clock = ui_create_label(ctx, 65, 23, "12:34 PM");
    ui_set_colors(clock, VGA_COLOR_WHITE, VGA_COLOR_DARK_GREY, VGA_COLOR_DARK_GREY);
    
    // Create a few desktop windows
    ui_component_t* window1 = ui_create_window(ctx, 5, 3, 30, 15, "File Explorer");
    ui_set_colors(window1, VGA_COLOR_BLACK, VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    
    // Add some content to the file explorer
    ui_component_t* file_list = ui_create_label(ctx, 7, 6, "Documents/");
    ui_set_colors(file_list, VGA_COLOR_BLUE, VGA_COLOR_WHITE, VGA_COLOR_WHITE);
    
    ui_component_t* file1 = ui_create_label(ctx, 7, 7, "  readme.txt");
    ui_component_t* file2 = ui_create_label(ctx, 7, 8, "  project.raeen");
    ui_component_t* file3 = ui_create_label(ctx, 7, 9, "  notes.md");
    
    // Create terminal window
    ui_component_t* terminal = ui_create_window(ctx, 40, 5, 35, 12, "RaeenShell");
    ui_set_colors(terminal, VGA_COLOR_GREEN, VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY);
    
    // Terminal content
    ui_component_t* prompt = ui_create_label(ctx, 42, 8, "raeen@raeenos:~$ ls -la");
    ui_set_colors(prompt, VGA_COLOR_GREEN, VGA_COLOR_BLACK, VGA_COLOR_BLACK);
    
    ui_component_t* output1 = ui_create_label(ctx, 42, 9, "drwxr-xr-x 2 raeen users");
    ui_component_t* output2 = ui_create_label(ctx, 42, 10, "-rw-r--r-- 1 raeen users");
    ui_set_colors(output1, VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK, VGA_COLOR_BLACK);
    ui_set_colors(output2, VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK, VGA_COLOR_BLACK);
    
    // Demo control
    ui_component_t* next_demo = ui_create_button(ctx, 60, 1, 15, "Next Demo");
    ui_set_colors(next_demo, VGA_COLOR_WHITE, VGA_COLOR_RED, VGA_COLOR_DARK_GREY);
    ui_set_event_handler(next_demo, demo_button_handler);
    
    // Status message
    ui_component_t* status = ui_create_label(ctx, 2, 1, "Demo: Desktop Environment - Drag windows, click buttons!");
    ui_set_colors(status, VGA_COLOR_YELLOW, VGA_COLOR_BLUE, VGA_COLOR_BLUE);
}

// ============================================================================
// GAMING OVERLAY DEMO
// ============================================================================

void ui_demo_gaming_overlay(ui_context_t* ctx) {
    // Gaming background (dark for contrast)
    vga_clear_with_color(VGA_COLOR_BLACK);
    
    // Simulate game viewport
    vga_fill_area(10, 5, 60, 15, '.', VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    vga_draw_box(10, 5, 60, 15, VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    
    // Game title
    ui_component_t* game_title = ui_create_label(ctx, 30, 6, "RAEEN QUEST 2025");
    ui_set_colors(game_title, VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK, VGA_COLOR_BLACK);
    
    // HUD Elements - Health bar
    ui_component_t* health_label = ui_create_label(ctx, 2, 2, "HP:");
    ui_set_colors(health_label, VGA_COLOR_RED, VGA_COLOR_BLACK, VGA_COLOR_BLACK);
    
    ui_component_t* health_bar = ui_create_progressbar(ctx, 6, 2, 20, 85);
    ui_set_colors(health_bar, VGA_COLOR_RED, VGA_COLOR_BLACK, VGA_COLOR_DARK_GREY);
    
    // Mana bar
    ui_component_t* mana_label = ui_create_label(ctx, 30, 2, "MP:");
    ui_set_colors(mana_label, VGA_COLOR_BLUE, VGA_COLOR_BLACK, VGA_COLOR_BLACK);
    
    ui_component_t* mana_bar = ui_create_progressbar(ctx, 34, 2, 20, 60);
    ui_set_colors(mana_bar, VGA_COLOR_BLUE, VGA_COLOR_BLACK, VGA_COLOR_DARK_GREY);
    
    // FPS Counter (ultra-low latency display)
    ui_component_t* fps_counter = ui_create_label(ctx, 70, 2, "FPS: 120");
    ui_set_colors(fps_counter, VGA_COLOR_YELLOW, VGA_COLOR_BLACK, VGA_COLOR_BLACK);
    
    // Frame time
    ui_component_t* frame_time = ui_create_label(ctx, 70, 3, "8.3ms");
    ui_set_colors(frame_time, VGA_COLOR_GREEN, VGA_COLOR_BLACK, VGA_COLOR_BLACK);
    
    // Minimap
    ui_component_t* minimap = ui_create_panel(ctx, 72, 8, 6, 6);
    ui_set_colors(minimap, VGA_COLOR_WHITE, VGA_COLOR_DARK_GREY, VGA_COLOR_WHITE);
    vga_fill_area(73, 9, 4, 4, 'M', VGA_COLOR_GREEN, VGA_COLOR_DARK_GREY);
    
    // Chat window (gaming communication)
    ui_component_t* chat_window = ui_create_panel(ctx, 2, 18, 40, 5);
    ui_set_colors(chat_window, VGA_COLOR_WHITE, VGA_COLOR_BLACK, VGA_COLOR_BLUE);
    
    ui_component_t* chat1 = ui_create_label(ctx, 3, 19, "Player1: Ready for raid!");
    ui_component_t* chat2 = ui_create_label(ctx, 3, 20, "AI_Assistant: Optimal route calculated");
    ui_set_colors(chat1, VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK, VGA_COLOR_BLACK);
    ui_set_colors(chat2, VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK, VGA_COLOR_BLACK);
    
    // Performance toggle
    ui_component_t* perf_btn = ui_create_button(ctx, 50, 22, 12, "Performance");
    ui_set_colors(perf_btn, VGA_COLOR_BLACK, VGA_COLOR_YELLOW, VGA_COLOR_DARK_GREY);
    ui_set_event_handler(perf_btn, demo_button_handler);
    
    // Next demo button
    ui_component_t* next_demo = ui_create_button(ctx, 60, 1, 15, "Next Demo");
    ui_set_colors(next_demo, VGA_COLOR_WHITE, VGA_COLOR_RED, VGA_COLOR_DARK_GREY);
    ui_set_event_handler(next_demo, demo_button_handler);
    
    // Status
    ui_component_t* status = ui_create_label(ctx, 2, 1, "Demo: Gaming Overlay - Ultra-low latency HUD elements");
    ui_set_colors(status, VGA_COLOR_YELLOW, VGA_COLOR_BLACK, VGA_COLOR_BLACK);
}

// ============================================================================
// AI ASSISTANCE DEMO
// ============================================================================

void ui_demo_ai_assistance(ui_context_t* ctx) {
    // AI-themed background
    vga_clear_with_color(VGA_COLOR_DARK_GREY);
    
    // AI Assistant window
    ui_component_t* ai_window = ui_create_window(ctx, 10, 4, 60, 18, "Rae AI Assistant");
    ui_set_colors(ai_window, VGA_COLOR_WHITE, VGA_COLOR_BLUE, VGA_COLOR_LIGHT_CYAN);
    
    // AI Avatar representation
    vga_fill_area(15, 8, 8, 6, ' ', VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLUE);
    vga_draw_box(15, 8, 8, 6, VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLUE);
    ui_component_t* ai_face = ui_create_label(ctx, 17, 10, "o o");
    ui_component_t* ai_mouth = ui_create_label(ctx, 18, 11, "^");
    ui_set_colors(ai_face, VGA_COLOR_WHITE, VGA_COLOR_BLUE, VGA_COLOR_BLUE);
    ui_set_colors(ai_mouth, VGA_COLOR_WHITE, VGA_COLOR_BLUE, VGA_COLOR_BLUE);
    
    // AI Conversation
    ui_component_t* ai_chat = ui_create_panel(ctx, 25, 8, 40, 10);
    ui_set_colors(ai_chat, VGA_COLOR_BLACK, VGA_COLOR_WHITE, VGA_COLOR_LIGHT_GREY);
    
    ui_component_t* ai_msg1 = ui_create_label(ctx, 26, 9, "AI: I notice you're working on");
    ui_component_t* ai_msg2 = ui_create_label(ctx, 26, 10, "    a kernel project. Would you");
    ui_component_t* ai_msg3 = ui_create_label(ctx, 26, 11, "    like me to optimize memory");
    ui_component_t* ai_msg4 = ui_create_label(ctx, 26, 12, "    allocation patterns?");
    ui_set_colors(ai_msg1, VGA_COLOR_BLUE, VGA_COLOR_WHITE, VGA_COLOR_WHITE);
    ui_set_colors(ai_msg2, VGA_COLOR_BLUE, VGA_COLOR_WHITE, VGA_COLOR_WHITE);
    ui_set_colors(ai_msg3, VGA_COLOR_BLUE, VGA_COLOR_WHITE, VGA_COLOR_WHITE);
    ui_set_colors(ai_msg4, VGA_COLOR_BLUE, VGA_COLOR_WHITE, VGA_COLOR_WHITE);
    
    ui_component_t* user_msg = ui_create_label(ctx, 26, 14, "User: Yes, analyze heap usage");
    ui_set_colors(user_msg, VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY, VGA_COLOR_LIGHT_GREY);
    
    ui_component_t* ai_response = ui_create_label(ctx, 26, 15, "AI: Analyzing... Found 3 optimizations!");
    ui_set_colors(ai_response, VGA_COLOR_GREEN, VGA_COLOR_WHITE, VGA_COLOR_WHITE);
    
    // AI suggestion buttons
    ui_component_t* suggest1 = ui_create_button(ctx, 12, 19, 20, "Auto-optimize Memory");
    ui_component_t* suggest2 = ui_create_button(ctx, 35, 19, 18, "Generate Tests");
    ui_component_t* suggest3 = ui_create_button(ctx, 55, 19, 15, "Code Review");
    
    ui_set_colors(suggest1, VGA_COLOR_WHITE, VGA_COLOR_GREEN, VGA_COLOR_DARK_GREY);
    ui_set_colors(suggest2, VGA_COLOR_WHITE, VGA_COLOR_CYAN, VGA_COLOR_DARK_GREY);
    ui_set_colors(suggest3, VGA_COLOR_WHITE, VGA_COLOR_MAGENTA, VGA_COLOR_DARK_GREY);
    
    // AI toggle button
    ui_component_t* ai_toggle = ui_create_button(ctx, 2, 2, 10, demo_state.ai_suggestions_enabled ? "AI: ON" : "AI: OFF");
    ui_set_colors(ai_toggle, VGA_COLOR_WHITE, demo_state.ai_suggestions_enabled ? VGA_COLOR_GREEN : VGA_COLOR_RED, VGA_COLOR_DARK_GREY);
    ui_set_event_handler(ai_toggle, demo_button_handler);
    
    // Context awareness indicator
    ui_component_t* context = ui_create_label(ctx, 2, 4, "Context: Kernel Development");
    ui_set_colors(context, VGA_COLOR_LIGHT_CYAN, VGA_COLOR_DARK_GREY, VGA_COLOR_DARK_GREY);
    
    ui_component_t* confidence = ui_create_label(ctx, 2, 5, "Confidence: 94%");
    ui_set_colors(confidence, VGA_COLOR_LIGHT_GREEN, VGA_COLOR_DARK_GREY, VGA_COLOR_DARK_GREY);
    
    // Next demo button
    ui_component_t* next_demo = ui_create_button(ctx, 60, 1, 15, "Next Demo");
    ui_set_colors(next_demo, VGA_COLOR_WHITE, VGA_COLOR_RED, VGA_COLOR_DARK_GREY);
    ui_set_event_handler(next_demo, demo_button_handler);
    
    // Status
    ui_component_t* status = ui_create_label(ctx, 2, 1, "Demo: AI Assistant - Context-aware intelligent suggestions");
    ui_set_colors(status, VGA_COLOR_YELLOW, VGA_COLOR_DARK_GREY, VGA_COLOR_DARK_GREY);
}

// ============================================================================
// ANIMATION SHOWCASE DEMO
// ============================================================================

void ui_demo_animations(ui_context_t* ctx) {
    // Animation demo background
    vga_clear_with_color(VGA_COLOR_BLACK);
    
    // Animated title (blinking effect)
    demo_state.demo_timer++;
    bool blink_state = (demo_state.demo_timer / 10) % 2;
    
    ui_component_t* title = ui_create_label(ctx, 25, 3, "ANIMATION SHOWCASE");
    ui_set_colors(title, blink_state ? VGA_COLOR_WHITE : VGA_COLOR_YELLOW, VGA_COLOR_BLACK, VGA_COLOR_BLACK);
    
    // Bouncing box simulation
    int bounce_pos = 15 + (demo_state.demo_timer / 5) % 20;
    if (((demo_state.demo_timer / 5) % 40) > 20) {
        bounce_pos = 35 - (bounce_pos - 15);
    }
    
    ui_component_t* bounce_box = ui_create_panel(ctx, bounce_pos, 8, 6, 4);
    ui_set_colors(bounce_box, VGA_COLOR_WHITE, VGA_COLOR_RED, VGA_COLOR_YELLOW);
    
    // Progress bar animation
    int progress = (demo_state.demo_timer / 2) % 100;
    ui_component_t* progress_label = ui_create_label(ctx, 20, 15, "Loading System:");
    ui_component_t* progress_bar = ui_create_progressbar(ctx, 35, 15, 30, progress);
    ui_set_colors(progress_bar, VGA_COLOR_GREEN, VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY);
    
    // Rotating character (simple rotation simulation)
    char* rotation_chars = "|/-\\";
    char rotation_char = rotation_chars[(demo_state.demo_timer / 3) % 4];
    char rotation_str[2] = {rotation_char, '\0'};
    
    ui_component_t* spinner = ui_create_label(ctx, 40, 12, rotation_str);
    ui_set_colors(spinner, VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK, VGA_COLOR_BLACK);
    
    // Color cycling demonstration
    vga_color cycle_colors[] = {VGA_COLOR_RED, VGA_COLOR_GREEN, VGA_COLOR_BLUE, VGA_COLOR_YELLOW, VGA_COLOR_MAGENTA, VGA_COLOR_CYAN};
    vga_color current_color = cycle_colors[(demo_state.demo_timer / 8) % 6];
    
    ui_component_t* color_box = ui_create_panel(ctx, 60, 8, 8, 4);
    ui_set_colors(color_box, VGA_COLOR_WHITE, current_color, VGA_COLOR_WHITE);
    
    // Fade effect simulation (using different intensities)
    int fade_level = (demo_state.demo_timer / 4) % 8;
    vga_color fade_colors[] = {VGA_COLOR_BLACK, VGA_COLOR_DARK_GREY, VGA_COLOR_LIGHT_GREY, VGA_COLOR_WHITE, 
                              VGA_COLOR_WHITE, VGA_COLOR_LIGHT_GREY, VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK};
    
    ui_component_t* fade_text = ui_create_label(ctx, 30, 18, "FADING TEXT");
    ui_set_colors(fade_text, fade_colors[fade_level], VGA_COLOR_BLACK, VGA_COLOR_BLACK);
    
    // Animation controls
    ui_component_t* anim_info = ui_create_label(ctx, 2, 22, "Animations: Bounce, Fade, Rotate, Color Cycle");
    ui_set_colors(anim_info, VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK, VGA_COLOR_BLACK);
    
    // Next demo button (back to first)
    ui_component_t* next_demo = ui_create_button(ctx, 60, 1, 15, "First Demo");
    ui_set_colors(next_demo, VGA_COLOR_WHITE, VGA_COLOR_GREEN, VGA_COLOR_DARK_GREY);
    ui_set_event_handler(next_demo, demo_button_handler);
    
    // Status
    ui_component_t* status = ui_create_label(ctx, 2, 1, "Demo: Animations - 60+ FPS smooth transitions and effects");
    ui_set_colors(status, VGA_COLOR_YELLOW, VGA_COLOR_BLACK, VGA_COLOR_BLACK);
}

// ============================================================================
// MAIN DEMO ORCHESTRATOR
// ============================================================================

void ui_run_revolutionary_demo(ui_context_t* ctx) {
    if (!ctx) return;
    
    // Clear all components for new demo
    for (int i = ctx->component_count - 1; i >= 0; i--) {
        ui_destroy_component(ctx, ctx->components[i]);
    }
    
    // Run appropriate demo
    switch (demo_state.current_demo) {
        case 0:
            ui_demo_desktop_environment(ctx);
            break;
        case 1:
            ui_demo_gaming_overlay(ctx);
            break;
        case 2:
            ui_demo_ai_assistance(ctx);
            break;
        case 3:
            ui_demo_animations(ctx);
            break;
        default:
            demo_state.current_demo = 0;
            ui_demo_desktop_environment(ctx);
            break;
    }
    
    // Show performance overlay if enabled
    if (demo_state.performance_overlay_visible) {
        ui_show_performance_overlay(ctx, true);
    }
    
    // Update demo timer
    demo_state.demo_timer++;
}

// Initialize demo state
void ui_init_demo(void) {
    demo_state.current_demo = 0;
    demo_state.demo_timer = 0;
    demo_state.ai_suggestions_enabled = true;
    demo_state.gaming_fps = 120;
    demo_state.system_load = 25;
    demo_state.performance_overlay_visible = false;
}