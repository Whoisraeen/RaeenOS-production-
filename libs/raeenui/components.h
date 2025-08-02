#pragma once

/**
 * RaeenUI Components - Declarative UI Primitives
 * Inspired by React, SwiftUI, and Flutter
 */

#include "raeenui_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// DECLARATIVE COMPONENT BUILDERS
// ============================================================================

// View component (basic container)
typedef struct {
    RaeenUIColor background_color;
    float corner_radius;
    float border_width;
    RaeenUIColor border_color;
    bool clip_children;
} RaeenUIViewProps;

RaeenUINode* RaeenUI_View(RaeenUIContext* ctx, RaeenUIViewProps props);

// Text component
typedef struct {
    char* text;
    float font_size;
    RaeenUIColor color;
    char* font_family;
    enum {
        RAEENUI_TEXT_ALIGN_LEFT,
        RAEENUI_TEXT_ALIGN_CENTER,
        RAEENUI_TEXT_ALIGN_RIGHT
    } text_align;
    bool bold;
    bool italic;
} RaeenUITextProps;

RaeenUINode* RaeenUI_Text(RaeenUIContext* ctx, RaeenUITextProps props);

// Button component
typedef struct {
    char* title;
    RaeenUIColor background_color;
    RaeenUIColor text_color;
    RaeenUIColor hover_color;
    RaeenUIColor pressed_color;
    float corner_radius;
    RaeenUIEventHandler on_click;
    void* user_data;
} RaeenUIButtonProps;

RaeenUINode* RaeenUI_Button(RaeenUIContext* ctx, RaeenUIButtonProps props);

// Input field component
typedef struct {
    char* placeholder;
    char* value;
    RaeenUIColor background_color;
    RaeenUIColor text_color;
    RaeenUIColor border_color;
    float corner_radius;
    bool is_password;
    bool is_multiline;
    int max_length;
    void (*on_change)(const char* new_value, void* user_data);
    void* user_data;
} RaeenUIInputProps;

RaeenUINode* RaeenUI_Input(RaeenUIContext* ctx, RaeenUIInputProps props);

// Image component
typedef struct {
    uint32_t* image_data;
    int width;
    int height;
    enum {
        RAEENUI_IMAGE_SCALE_FIT,
        RAEENUI_IMAGE_SCALE_FILL,
        RAEENUI_IMAGE_SCALE_STRETCH
    } scale_mode;
    float opacity;
    float corner_radius;
} RaeenUIImageProps;

RaeenUINode* RaeenUI_Image(RaeenUIContext* ctx, RaeenUIImageProps props);

// Scroll view component
typedef struct {
    bool horizontal_scroll;
    bool vertical_scroll;
    bool show_scrollbars;
    RaeenUIColor scrollbar_color;
} RaeenUIScrollViewProps;

RaeenUINode* RaeenUI_ScrollView(RaeenUIContext* ctx, RaeenUIScrollViewProps props);

// ============================================================================
// LAYOUT COMPONENTS
// ============================================================================

// Stack component (VStack/HStack from SwiftUI)
typedef struct {
    enum {
        RAEENUI_STACK_VERTICAL,
        RAEENUI_STACK_HORIZONTAL
    } direction;
    RaeenUIAlignment alignment;
    float spacing;
    float padding;
} RaeenUIStackProps;

RaeenUINode* RaeenUI_Stack(RaeenUIContext* ctx, RaeenUIStackProps props);

// Grid component
typedef struct {
    int columns;
    int rows;
    float column_spacing;
    float row_spacing;
    RaeenUIAlignment column_alignment;
    RaeenUIAlignment row_alignment;
} RaeenUIGridProps;

RaeenUINode* RaeenUI_Grid(RaeenUIContext* ctx, RaeenUIGridProps props);

// Flex component (CSS Flexbox)
typedef struct {
    RaeenUIFlexDirection direction;
    RaeenUIAlignment justify_content;
    RaeenUIAlignment align_items;
    bool wrap;
    float gap;
} RaeenUIFlexProps;

RaeenUINode* RaeenUI_Flex(RaeenUIContext* ctx, RaeenUIFlexProps props);

// ============================================================================
// ADVANCED COMPONENTS
// ============================================================================

// Window component
typedef struct {
    char* title;
    bool resizable;
    bool closable;
    bool minimizable;
    bool maximizable;
    RaeenUIVec2 min_size;
    RaeenUIVec2 max_size;
    RaeenUIColor title_bar_color;
    RaeenUIColor title_text_color;
    void (*on_close)(void* user_data);
    void* user_data;
} RaeenUIWindowProps;

RaeenUINode* RaeenUI_Window(RaeenUIContext* ctx, RaeenUIWindowProps props);

// Menu component
typedef struct {
    char** items;
    int item_count;
    int selected_index;
    RaeenUIColor background_color;
    RaeenUIColor selected_color;
    RaeenUIColor text_color;
    void (*on_select)(int index, void* user_data);
    void* user_data;
} RaeenUIMenuProps;

RaeenUINode* RaeenUI_Menu(RaeenUIContext* ctx, RaeenUIMenuProps props);

// Slider component
typedef struct {
    float min_value;
    float max_value;
    float current_value;
    RaeenUIColor track_color;
    RaeenUIColor thumb_color;
    RaeenUIColor fill_color;
    bool vertical;
    void (*on_change)(float value, void* user_data);
    void* user_data;
} RaeenUISliderProps;

RaeenUINode* RaeenUI_Slider(RaeenUIContext* ctx, RaeenUISliderProps props);

// ============================================================================
// DECLARATIVE STYLE BUILDERS
// ============================================================================

// Style builder pattern for fluent API
typedef struct {
    RaeenUIStyle style;
} RaeenUIStyleBuilder;

RaeenUIStyleBuilder* RaeenUI_Style(void);
RaeenUIStyleBuilder* RaeenUI_Width(RaeenUIStyleBuilder* builder, float width);
RaeenUIStyleBuilder* RaeenUI_Height(RaeenUIStyleBuilder* builder, float height);
RaeenUIStyleBuilder* RaeenUI_Padding(RaeenUIStyleBuilder* builder, float padding);
RaeenUIStyleBuilder* RaeenUI_Margin(RaeenUIStyleBuilder* builder, float margin);
RaeenUIStyleBuilder* RaeenUI_BackgroundColor(RaeenUIStyleBuilder* builder, RaeenUIColor color);
RaeenUIStyleBuilder* RaeenUI_BorderRadius(RaeenUIStyleBuilder* builder, float radius);
RaeenUIStyleBuilder* RaeenUI_Opacity(RaeenUIStyleBuilder* builder, float opacity);
RaeenUIStyle RaeenUI_BuildStyle(RaeenUIStyleBuilder* builder);

// ============================================================================
// MACROS FOR DECLARATIVE SYNTAX
// ============================================================================

// Helper macros for color creation
#define RAEENUI_COLOR_RGB(r, g, b) ((RaeenUIColor){(r)/255.0f, (g)/255.0f, (b)/255.0f, 1.0f})
#define RAEENUI_COLOR_RGBA(r, g, b, a) ((RaeenUIColor){(r)/255.0f, (g)/255.0f, (b)/255.0f, (a)/255.0f})
#define RAEENUI_COLOR_WHITE RAEENUI_COLOR_RGB(255, 255, 255)
#define RAEENUI_COLOR_BLACK RAEENUI_COLOR_RGB(0, 0, 0)
#define RAEENUI_COLOR_TRANSPARENT RAEENUI_COLOR_RGBA(0, 0, 0, 0)
#define RAEENUI_COLOR_BLUE RAEENUI_COLOR_RGB(0, 122, 255)
#define RAEENUI_COLOR_RED RAEENUI_COLOR_RGB(255, 59, 48)
#define RAEENUI_COLOR_GREEN RAEENUI_COLOR_RGB(52, 199, 89)

// Helper macros for vector creation
#define RAEENUI_VEC2(x, y) ((RaeenUIVec2){(x), (y)})
#define RAEENUI_RECT(x, y, w, h) ((RaeenUIRect){RAEENUI_VEC2(x, y), RAEENUI_VEC2(w, h)})

// Macro for easy component tree building
#define RAEENUI_CHILDREN(...) __VA_ARGS__

// ============================================================================
// HIGH-LEVEL SCENE BUILDERS
// ============================================================================

// Desktop environment components
RaeenUINode* RaeenUI_Desktop(RaeenUIContext* ctx);
RaeenUINode* RaeenUI_Taskbar(RaeenUIContext* ctx);
RaeenUINode* RaeenUI_StartMenu(RaeenUIContext* ctx);
RaeenUINode* RaeenUI_SystemTray(RaeenUIContext* ctx);
RaeenUINode* RaeenUI_NotificationCenter(RaeenUIContext* ctx);

// Gaming-optimized components
RaeenUINode* RaeenUI_GameOverlay(RaeenUIContext* ctx);
RaeenUINode* RaeenUI_PerformanceMonitor(RaeenUIContext* ctx);
RaeenUINode* RaeenUI_GameLauncher(RaeenUIContext* ctx);

// Developer tools
RaeenUINode* RaeenUI_Inspector(RaeenUIContext* ctx);
RaeenUINode* RaeenUI_Console(RaeenUIContext* ctx);
RaeenUINode* RaeenUI_Profiler(RaeenUIContext* ctx);

#ifdef __cplusplus
}
#endif