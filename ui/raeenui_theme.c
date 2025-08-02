/**
 * RaeenUI - Theme and Animation System
 * Advanced theming with CSS-like styling and smooth animations
 */

#include "raeenui.h"
#include "../memory.h"
#include "../string.h"
#include "../time.h"

// Animation interpolation functions
static float ease_linear(float t);
static float ease_in_quad(float t);
static float ease_out_quad(float t);
static float ease_in_out_quad(float t);
static float ease_bounce(float t);
static float ease_spring(float t);

// Theme management
static RaeenUITheme* g_builtin_themes[4] = {NULL};
static bool g_themes_initialized = false;

// Animation management
static RaeenUIAnimation* g_animation_pool = NULL;
static uint32_t g_animation_pool_size = 0;
static uint32_t g_next_animation_id = 1;

/**
 * Create a new theme
 */
RaeenUITheme* raeenui_create_theme(const char* name, RaeenUIThemeMode mode) {
    if (!name) return NULL;
    
    RaeenUITheme* theme = (RaeenUITheme*)memory_alloc(sizeof(RaeenUITheme));
    if (!theme) return NULL;
    
    memory_set(theme, 0, sizeof(RaeenUITheme));
    
    string_copy(theme->name, name, sizeof(theme->name));
    theme->mode = mode;
    
    // Set default values
    theme->base_font_size = 14.0f;
    theme->base_spacing = 8.0f;
    theme->base_corner_radius = 4.0f;
    theme->base_border_width = 1.0f;
    theme->default_blur_radius = 10.0f;
    theme->default_shadow_offset = 2.0f;
    theme->default_shadow_blur = 4.0f;
    theme->default_shadow_color = raeenui_color_rgba(0, 0, 0, 0.3f);
    
    string_copy(theme->primary_font, "RaeenUI-Regular", sizeof(theme->primary_font));
    string_copy(theme->secondary_font, "RaeenUI-Light", sizeof(theme->secondary_font));
    
    return theme;
}

/**
 * Destroy a theme
 */
void raeenui_destroy_theme(RaeenUITheme* theme) {
    if (!theme) return;
    
    if (theme->custom_data) {
        memory_free(theme->custom_data);
    }
    
    memory_free(theme);
}

/**
 * Set active theme for context
 */
void raeenui_set_theme(RaeenUIContext* context, RaeenUITheme* theme) {
    if (!context || !theme) return;
    
    context->current_theme = theme;
    
    // Mark all windows for redraw
    RaeenUIWindow* window = context->windows;
    while (window) {
        window->theme = theme;
        window->needs_redraw = true;
        window = window->next;
    }
    
    printf("RaeenUI: Theme changed to '%s'\n", theme->name);
}

/**
 * Get builtin theme
 */
RaeenUITheme* raeenui_get_builtin_theme(RaeenUIThemeMode mode) {
    if (!g_themes_initialized) {
        raeenui_init_builtin_themes();
    }
    
    switch (mode) {
        case RAEENUI_THEME_LIGHT:
            return g_builtin_themes[0];
        case RAEENUI_THEME_DARK:
            return g_builtin_themes[1];
        case RAEENUI_THEME_AUTO:
            // Return light or dark based on system preference
            return g_builtin_themes[0]; // Default to light
        default:
            return g_builtin_themes[0];
    }
}

/**
 * Initialize builtin themes
 */
void raeenui_init_builtin_themes(void) {
    if (g_themes_initialized) return;
    
    // Light Theme (macOS/iOS inspired)
    RaeenUITheme* light = raeenui_create_theme("RaeenUI Light", RAEENUI_THEME_LIGHT);
    if (light) {
        light->primary = raeenui_color_hex(0x007AFF);        // iOS Blue
        light->secondary = raeenui_color_hex(0x5856D6);      // iOS Purple
        light->accent = raeenui_color_hex(0xFF3B30);         // iOS Red
        light->background = raeenui_color_hex(0xFFFFFF);     // White
        light->surface = raeenui_color_hex(0xF2F2F7);        // Light Gray
        light->error = raeenui_color_hex(0xFF3B30);          // Red
        light->warning = raeenui_color_hex(0xFF9500);        // Orange
        light->success = raeenui_color_hex(0x34C759);        // Green
        light->text_primary = raeenui_color_hex(0x000000);   // Black
        light->text_secondary = raeenui_color_hex(0x3C3C43); // Dark Gray
        g_builtin_themes[0] = light;
    }
    
    // Dark Theme (macOS/iOS Dark Mode inspired)
    RaeenUITheme* dark = raeenui_create_theme("RaeenUI Dark", RAEENUI_THEME_DARK);
    if (dark) {
        dark->primary = raeenui_color_hex(0x0A84FF);         // iOS Blue (Dark)
        dark->secondary = raeenui_color_hex(0x5E5CE6);       // iOS Purple (Dark)
        dark->accent = raeenui_color_hex(0xFF453A);          // iOS Red (Dark)
        dark->background = raeenui_color_hex(0x000000);      // Black
        dark->surface = raeenui_color_hex(0x1C1C1E);         // Dark Gray
        dark->error = raeenui_color_hex(0xFF453A);           // Red (Dark)
        dark->warning = raeenui_color_hex(0xFF9F0A);         // Orange (Dark)
        dark->success = raeenui_color_hex(0x30D158);         // Green (Dark)
        dark->text_primary = raeenui_color_hex(0xFFFFFF);    // White
        dark->text_secondary = raeenui_color_hex(0xEBEBF5);  // Light Gray
        g_builtin_themes[1] = dark;
    }
    
    // Windows 11 Fluent Theme
    RaeenUITheme* fluent = raeenui_create_theme("Fluent Design", RAEENUI_THEME_LIGHT);
    if (fluent) {
        fluent->primary = raeenui_color_hex(0x0078D4);       // Windows Blue
        fluent->secondary = raeenui_color_hex(0x8764B8);     // Windows Purple
        fluent->accent = raeenui_color_hex(0xD13438);        // Windows Red
        fluent->background = raeenui_color_hex(0xFAFAFA);    // Light Background
        fluent->surface = raeenui_color_hex(0xF3F3F3);       // Surface
        fluent->text_primary = raeenui_color_hex(0x323130);  // Dark Text
        fluent->text_secondary = raeenui_color_hex(0x605E5C); // Secondary Text
        fluent->base_corner_radius = 8.0f;                   // Rounded corners
        fluent->default_blur_radius = 20.0f;                 // Acrylic blur
        g_builtin_themes[2] = fluent;
    }
    
    // GNOME Adwaita Theme
    RaeenUITheme* adwaita = raeenui_create_theme("Adwaita", RAEENUI_THEME_LIGHT);
    if (adwaita) {
        adwaita->primary = raeenui_color_hex(0x3584E4);      // GNOME Blue
        adwaita->secondary = raeenui_color_hex(0x9141AC);    // GNOME Purple
        adwaita->accent = raeenui_color_hex(0xE01B24);       // GNOME Red
        adwaita->background = raeenui_color_hex(0xFAFAFA);   // Light Background
        adwaita->surface = raeenui_color_hex(0xFFFFFF);      // White Surface
        adwaita->text_primary = raeenui_color_hex(0x2E3436); // Dark Text
        adwaita->text_secondary = raeenui_color_hex(0x555753); // Secondary Text
        adwaita->base_corner_radius = 6.0f;                  // Subtle rounding
        g_builtin_themes[3] = adwaita;
    }
    
    g_themes_initialized = true;
    printf("RaeenUI: Builtin themes initialized\n");
}

/**
 * Apply theme to view
 */
void raeenui_apply_theme_to_view(RaeenUIView* view, RaeenUITheme* theme) {
    if (!view || !theme) return;
    
    // Apply theme colors based on view type
    switch (view->type) {
        case RAEENUI_VIEW_BUTTON:
            view->style.background_color = theme->primary;
            view->style.foreground_color = theme->background;
            view->style.corner_radius = theme->base_corner_radius;
            break;
            
        case RAEENUI_VIEW_TEXT:
            view->style.foreground_color = theme->text_primary;
            view->style.font_size = theme->base_font_size;
            string_copy(view->style.font_family, theme->primary_font, sizeof(view->style.font_family));
            break;
            
        case RAEENUI_VIEW_CONTAINER:
            view->style.background_color = theme->surface;
            break;
            
        default:
            view->style.background_color = theme->background;
            view->style.foreground_color = theme->text_primary;
            break;
    }
    
    // Apply spacing and sizing
    view->style.padding.top = theme->base_spacing;
    view->style.padding.bottom = theme->base_spacing;
    view->style.padding.left = theme->base_spacing * 1.5f;
    view->style.padding.right = theme->base_spacing * 1.5f;
    
    view->needs_render = true;
}

/**
 * Create animation
 */
RaeenUIAnimation* raeenui_create_animation(RaeenUIView* target, float duration) {
    if (!target || duration <= 0.0f) return NULL;
    
    RaeenUIAnimation* anim = (RaeenUIAnimation*)memory_alloc(sizeof(RaeenUIAnimation));
    if (!anim) return NULL;
    
    memory_set(anim, 0, sizeof(RaeenUIAnimation));
    
    anim->animation_id = g_next_animation_id++;
    anim->target_view = target;
    anim->duration = duration;
    anim->curve = RAEENUI_ANIM_EASE_IN_OUT;
    anim->from_frame = target->frame;
    anim->to_frame = target->frame;
    anim->from_opacity = target->style.opacity;
    anim->to_opacity = target->style.opacity;
    anim->from_color = target->style.background_color;
    anim->to_color = target->style.background_color;
    
    return anim;
}

/**
 * Start animation
 */
void raeenui_start_animation(RaeenUIAnimation* animation) {
    if (!animation || animation->is_running) return;
    
    animation->current_time = 0.0f;
    animation->is_running = true;
    animation->is_paused = false;
    
    if (animation->on_start) {
        animation->on_start(animation);
    }
    
    printf("RaeenUI: Animation %u started\n", animation->animation_id);
}

/**
 * Stop animation
 */
void raeenui_stop_animation(RaeenUIAnimation* animation) {
    if (!animation || !animation->is_running) return;
    
    animation->is_running = false;
    animation->is_paused = false;
    
    if (animation->on_complete) {
        animation->on_complete(animation);
    }
    
    printf("RaeenUI: Animation %u stopped\n", animation->animation_id);
}

/**
 * Update all animations
 */
void raeenui_update_animations(RaeenUIContext* context, float delta_time) {
    if (!context) return;
    
    RaeenUIAnimation* anim = context->active_animations;
    RaeenUIAnimation* prev = NULL;
    
    while (anim) {
        RaeenUIAnimation* next = anim->next;
        
        if (anim->is_running && !anim->is_paused) {
            anim->current_time += delta_time;
            
            // Calculate progress (0.0 to 1.0)
            float progress = anim->current_time / anim->duration;
            
            if (progress >= 1.0f) {
                progress = 1.0f;
                anim->is_running = false;
            }
            
            // Apply easing curve
            float eased_progress = raeenui_apply_easing(progress, anim->curve);
            
            // Interpolate properties
            raeenui_interpolate_animation_properties(anim, eased_progress);
            
            // Call update callback
            if (anim->on_update) {
                anim->on_update(anim, eased_progress);
            }
            
            // Mark target view for redraw
            if (anim->target_view) {
                anim->target_view->needs_render = true;
            }
            
            // Check if animation completed
            if (!anim->is_running) {
                if (anim->repeat) {
                    // Restart animation
                    anim->current_time = 0.0f;
                    anim->is_running = true;
                    
                    if (anim->auto_reverse) {
                        // Swap from/to values
                        RaeenUIRect temp_frame = anim->from_frame;
                        anim->from_frame = anim->to_frame;
                        anim->to_frame = temp_frame;
                        
                        float temp_opacity = anim->from_opacity;
                        anim->from_opacity = anim->to_opacity;
                        anim->to_opacity = temp_opacity;
                        
                        RaeenUIColor temp_color = anim->from_color;
                        anim->from_color = anim->to_color;
                        anim->to_color = temp_color;
                    }
                } else {
                    // Animation completed
                    if (anim->on_complete) {
                        anim->on_complete(anim);
                    }
                    
                    // Remove from active list
                    if (prev) {
                        prev->next = next;
                    } else {
                        context->active_animations = next;
                    }
                    
                    context->animation_count--;
                    memory_free(anim);
                    anim = NULL;
                }
            }
        }
        
        if (anim) {
            prev = anim;
        }
        anim = next;
    }
}

/**
 * Apply easing curve to progress
 */
float raeenui_apply_easing(float progress, RaeenUIAnimationCurve curve) {
    switch (curve) {
        case RAEENUI_ANIM_LINEAR:
            return ease_linear(progress);
        case RAEENUI_ANIM_EASE_IN:
            return ease_in_quad(progress);
        case RAEENUI_ANIM_EASE_OUT:
            return ease_out_quad(progress);
        case RAEENUI_ANIM_EASE_IN_OUT:
            return ease_in_out_quad(progress);
        case RAEENUI_ANIM_BOUNCE:
            return ease_bounce(progress);
        case RAEENUI_ANIM_SPRING:
            return ease_spring(progress);
        default:
            return progress;
    }
}

/**
 * Interpolate animation properties
 */
void raeenui_interpolate_animation_properties(RaeenUIAnimation* anim, float progress) {
    if (!anim || !anim->target_view) return;
    
    // Interpolate frame
    anim->target_view->frame.origin.x = anim->from_frame.origin.x + 
        (anim->to_frame.origin.x - anim->from_frame.origin.x) * progress;
    anim->target_view->frame.origin.y = anim->from_frame.origin.y + 
        (anim->to_frame.origin.y - anim->from_frame.origin.y) * progress;
    anim->target_view->frame.size.width = anim->from_frame.size.width + 
        (anim->to_frame.size.width - anim->from_frame.size.width) * progress;
    anim->target_view->frame.size.height = anim->from_frame.size.height + 
        (anim->to_frame.size.height - anim->from_frame.size.height) * progress;
    
    // Interpolate opacity
    anim->target_view->style.opacity = anim->from_opacity + 
        (anim->to_opacity - anim->from_opacity) * progress;
    
    // Interpolate color
    anim->target_view->style.background_color.r = anim->from_color.r + 
        (anim->to_color.r - anim->from_color.r) * progress;
    anim->target_view->style.background_color.g = anim->from_color.g + 
        (anim->to_color.g - anim->from_color.g) * progress;
    anim->target_view->style.background_color.b = anim->from_color.b + 
        (anim->to_color.b - anim->from_color.b) * progress;
    anim->target_view->style.background_color.a = anim->from_color.a + 
        (anim->to_color.a - anim->from_color.a) * progress;
}

// Easing function implementations

static float ease_linear(float t) {
    return t;
}

static float ease_in_quad(float t) {
    return t * t;
}

static float ease_out_quad(float t) {
    return 1.0f - (1.0f - t) * (1.0f - t);
}

static float ease_in_out_quad(float t) {
    return t < 0.5f ? 2.0f * t * t : 1.0f - 2.0f * (1.0f - t) * (1.0f - t);
}

static float ease_bounce(float t) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;
    
    if (t < 1.0f / d1) {
        return n1 * t * t;
    } else if (t < 2.0f / d1) {
        t -= 1.5f / d1;
        return n1 * t * t + 0.75f;
    } else if (t < 2.5f / d1) {
        t -= 2.25f / d1;
        return n1 * t * t + 0.9375f;
    } else {
        t -= 2.625f / d1;
        return n1 * t * t + 0.984375f;
    }
}

static float ease_spring(float t) {
    const float c4 = (2.0f * 3.14159f) / 3.0f;
    
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    
    return -powf(2.0f, 10.0f * (t - 1.0f)) * sinf((t - 1.1f) * c4);
}

// Math helper functions
static float powf(float base, float exp) {
    // Simple power function implementation
    if (exp == 0.0f) return 1.0f;
    if (exp == 1.0f) return base;
    if (exp == 2.0f) return base * base;
    
    // For other cases, use approximation
    float result = 1.0f;
    for (int i = 0; i < (int)exp; i++) {
        result *= base;
    }
    return result;
}

static float sinf(float x) {
    // Simple sine approximation using Taylor series
    float result = x;
    float term = x;
    
    for (int i = 1; i < 10; i++) {
        term *= -x * x / ((2 * i) * (2 * i + 1));
        result += term;
    }
    
    return result;
}
