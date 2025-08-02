/**
 * RaeenUI - Rendering System
 * GPU-accelerated rendering with modern effects
 */

#include "raeenui.h"
#include "../gpu/graphics_pipeline.h"
#include "../memory.h"
#include "../string.h"

// Shader programs for different view types
static uint32_t g_rect_shader = 0;
static uint32_t g_text_shader = 0;
static uint32_t g_image_shader = 0;
static uint32_t g_blur_shader = 0;
static uint32_t g_shadow_shader = 0;

// Vertex buffer for quad rendering
static uint32_t g_quad_vbo = 0;
static uint32_t g_quad_vao = 0;

// Font rendering resources
typedef struct {
    uint32_t texture_id;
    float advance_x;
    float advance_y;
    float bitmap_width;
    float bitmap_height;
    float bitmap_left;
    float bitmap_top;
} GlyphInfo;

static GlyphInfo g_glyph_cache[256]; // ASCII character cache
static uint32_t g_font_texture_atlas = 0;

// Internal function declarations
static void raeenui_init_shaders(void);
static void raeenui_init_quad_geometry(void);
static void raeenui_init_font_system(void);
static void raeenui_render_rect(RaeenUIView* view, GraphicsContext* gfx);
static void raeenui_render_text(RaeenUIView* view, GraphicsContext* gfx);
static void raeenui_render_image(RaeenUIView* view, GraphicsContext* gfx);
static void raeenui_render_button(RaeenUIView* view, GraphicsContext* gfx);
static void raeenui_apply_effects(RaeenUIView* view, GraphicsContext* gfx);
static void raeenui_render_shadow(RaeenUIView* view, GraphicsContext* gfx);
static void raeenui_render_blur(RaeenUIView* view, GraphicsContext* gfx);

/**
 * Initialize rendering system
 */
bool raeenui_init_rendering(void) {
    printf("RaeenUI: Initializing rendering system\n");
    
    raeenui_init_shaders();
    raeenui_init_quad_geometry();
    raeenui_init_font_system();
    
    printf("RaeenUI: Rendering system initialized\n");
    return true;
}

/**
 * Shutdown rendering system
 */
void raeenui_shutdown_rendering(void) {
    // Cleanup GPU resources
    if (g_quad_vbo) {
        graphics_delete_buffer(g_quad_vbo);
        g_quad_vbo = 0;
    }
    
    if (g_font_texture_atlas) {
        graphics_delete_texture(g_font_texture_atlas);
        g_font_texture_atlas = 0;
    }
    
    printf("RaeenUI: Rendering system shutdown\n");
}

/**
 * Render a view with GPU acceleration
 */
void raeenui_render_view(RaeenUIView* view, GraphicsContext* gfx) {
    if (!view || !gfx || !view->style.visible) return;
    
    // Set up transformation matrix for view
    float transform[16];
    graphics_matrix_identity(transform);
    graphics_matrix_translate(transform, view->frame.origin.x, view->frame.origin.y, 0.0f);
    graphics_set_transform_matrix(gfx, transform);
    
    // Apply opacity
    graphics_set_opacity(gfx, view->style.opacity);
    
    // Render shadow first if enabled
    if (view->style.shadow_blur > 0.0f) {
        raeenui_render_shadow(view, gfx);
    }
    
    // Render background
    if (view->style.background_color.a > 0.0f) {
        raeenui_render_rect(view, gfx);
    }
    
    // Render content based on view type
    switch (view->type) {
        case RAEENUI_VIEW_TEXT:
            raeenui_render_text(view, gfx);
            break;
            
        case RAEENUI_VIEW_BUTTON:
            raeenui_render_button(view, gfx);
            break;
            
        case RAEENUI_VIEW_IMAGE:
            raeenui_render_image(view, gfx);
            break;
            
        case RAEENUI_VIEW_CUSTOM:
            if (view->custom_render) {
                view->custom_render(view, gfx);
            }
            break;
            
        default:
            // Container views just render background
            break;
    }
    
    // Apply post-processing effects
    raeenui_apply_effects(view, gfx);
    
    view->needs_render = false;
}

/**
 * Measure view size for layout
 */
RaeenUISize raeenui_measure_view(RaeenUIView* view, RaeenUISize available_size) {
    if (!view) return raeenui_size_make(0, 0);
    
    RaeenUISize measured_size = view->layout.preferred_size;
    
    // Auto-size based on content
    if (measured_size.width <= 0 || measured_size.height <= 0) {
        switch (view->type) {
            case RAEENUI_VIEW_TEXT:
                if (view->text_content) {
                    // Measure text dimensions
                    float text_width = string_length(view->text_content) * view->style.font_size * 0.6f; // Rough estimate
                    float text_height = view->style.font_size * 1.2f;
                    
                    if (measured_size.width <= 0) measured_size.width = text_width;
                    if (measured_size.height <= 0) measured_size.height = text_height;
                }
                break;
                
            case RAEENUI_VIEW_BUTTON:
                if (view->text_content) {
                    float text_width = string_length(view->text_content) * view->style.font_size * 0.6f;
                    float text_height = view->style.font_size * 1.2f;
                    
                    if (measured_size.width <= 0) measured_size.width = text_width + 32.0f; // Add padding
                    if (measured_size.height <= 0) measured_size.height = text_height + 16.0f;
                }
                break;
                
            case RAEENUI_VIEW_IMAGE:
                if (measured_size.width <= 0) measured_size.width = view->image_width;
                if (measured_size.height <= 0) measured_size.height = view->image_height;
                break;
                
            default:
                // Use available size for containers
                if (measured_size.width <= 0) measured_size.width = available_size.width;
                if (measured_size.height <= 0) measured_size.height = available_size.height;
                break;
        }
    }
    
    // Apply size constraints
    if (view->layout.min_size.width > 0) {
        measured_size.width = fmaxf(measured_size.width, view->layout.min_size.width);
    }
    if (view->layout.min_size.height > 0) {
        measured_size.height = fmaxf(measured_size.height, view->layout.min_size.height);
    }
    if (view->layout.max_size.width > 0) {
        measured_size.width = fminf(measured_size.width, view->layout.max_size.width);
    }
    if (view->layout.max_size.height > 0) {
        measured_size.height = fminf(measured_size.height, view->layout.max_size.height);
    }
    
    // Add padding and margin
    measured_size.width += view->style.padding.left + view->style.padding.right;
    measured_size.height += view->style.padding.top + view->style.padding.bottom;
    measured_size.width += view->style.margin.left + view->style.margin.right;
    measured_size.height += view->style.margin.top + view->style.margin.bottom;
    
    return measured_size;
}

// Internal rendering functions

/**
 * Initialize GPU shaders
 */
static void raeenui_init_shaders(void) {
    // Rectangle shader (for backgrounds, borders)
    const char* rect_vertex_shader = 
        "#version 330 core\n"
        "layout (location = 0) in vec2 position;\n"
        "layout (location = 1) in vec2 texCoord;\n"
        "uniform mat4 transform;\n"
        "uniform vec2 size;\n"
        "out vec2 fragTexCoord;\n"
        "void main() {\n"
        "    vec2 scaledPos = position * size;\n"
        "    gl_Position = transform * vec4(scaledPos, 0.0, 1.0);\n"
        "    fragTexCoord = texCoord;\n"
        "}\n";
    
    const char* rect_fragment_shader =
        "#version 330 core\n"
        "in vec2 fragTexCoord;\n"
        "uniform vec4 color;\n"
        "uniform float cornerRadius;\n"
        "uniform vec2 size;\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "    vec2 pos = fragTexCoord * size;\n"
        "    vec2 center = size * 0.5;\n"
        "    float dist = length(max(abs(pos - center) - (center - cornerRadius), 0.0));\n"
        "    float alpha = 1.0 - smoothstep(cornerRadius - 1.0, cornerRadius, dist);\n"
        "    FragColor = vec4(color.rgb, color.a * alpha);\n"
        "}\n";
    
    g_rect_shader = graphics_create_shader_program(rect_vertex_shader, rect_fragment_shader);
    
    // Text shader
    const char* text_vertex_shader =
        "#version 330 core\n"
        "layout (location = 0) in vec2 position;\n"
        "layout (location = 1) in vec2 texCoord;\n"
        "uniform mat4 transform;\n"
        "uniform vec2 glyphPos;\n"
        "uniform vec2 glyphSize;\n"
        "out vec2 fragTexCoord;\n"
        "void main() {\n"
        "    vec2 pos = glyphPos + position * glyphSize;\n"
        "    gl_Position = transform * vec4(pos, 0.0, 1.0);\n"
        "    fragTexCoord = texCoord;\n"
        "}\n";
    
    const char* text_fragment_shader =
        "#version 330 core\n"
        "in vec2 fragTexCoord;\n"
        "uniform sampler2D fontTexture;\n"
        "uniform vec4 textColor;\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "    float alpha = texture(fontTexture, fragTexCoord).r;\n"
        "    FragColor = vec4(textColor.rgb, textColor.a * alpha);\n"
        "}\n";
    
    g_text_shader = graphics_create_shader_program(text_vertex_shader, text_fragment_shader);
    
    // Blur shader for effects
    const char* blur_fragment_shader =
        "#version 330 core\n"
        "in vec2 fragTexCoord;\n"
        "uniform sampler2D sourceTexture;\n"
        "uniform vec2 blurDirection;\n"
        "uniform float blurRadius;\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "    vec4 color = vec4(0.0);\n"
        "    float totalWeight = 0.0;\n"
        "    for (int i = -8; i <= 8; i++) {\n"
        "        float weight = exp(-float(i*i) / (2.0 * blurRadius * blurRadius));\n"
        "        vec2 offset = blurDirection * float(i) / textureSize(sourceTexture, 0);\n"
        "        color += texture(sourceTexture, fragTexCoord + offset) * weight;\n"
        "        totalWeight += weight;\n"
        "    }\n"
        "    FragColor = color / totalWeight;\n"
        "}\n";
    
    g_blur_shader = graphics_create_shader_program(rect_vertex_shader, blur_fragment_shader);
    
    printf("RaeenUI: Shaders initialized\n");
}

/**
 * Initialize quad geometry for rendering
 */
static void raeenui_init_quad_geometry(void) {
    // Quad vertices (position + texcoord)
    float quad_vertices[] = {
        0.0f, 0.0f,  0.0f, 0.0f,  // Bottom-left
        1.0f, 0.0f,  1.0f, 0.0f,  // Bottom-right
        1.0f, 1.0f,  1.0f, 1.0f,  // Top-right
        0.0f, 1.0f,  0.0f, 1.0f   // Top-left
    };
    
    uint32_t quad_indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    
    g_quad_vbo = graphics_create_vertex_buffer(quad_vertices, sizeof(quad_vertices));
    g_quad_vao = graphics_create_vertex_array();
    
    graphics_bind_vertex_array(g_quad_vao);
    graphics_bind_vertex_buffer(g_quad_vbo);
    
    // Position attribute
    graphics_vertex_attrib_pointer(0, 2, sizeof(float) * 4, 0);
    // Texture coordinate attribute
    graphics_vertex_attrib_pointer(1, 2, sizeof(float) * 4, sizeof(float) * 2);
    
    uint32_t ebo = graphics_create_index_buffer(quad_indices, sizeof(quad_indices));
    graphics_bind_index_buffer(ebo);
    
    printf("RaeenUI: Quad geometry initialized\n");
}

/**
 * Initialize font rendering system
 */
static void raeenui_init_font_system(void) {
    // Create a simple bitmap font atlas (placeholder)
    // In a real implementation, this would load TTF fonts and generate texture atlas
    
    uint32_t atlas_width = 512;
    uint32_t atlas_height = 512;
    uint8_t* atlas_data = (uint8_t*)memory_alloc(atlas_width * atlas_height);
    
    // Generate simple ASCII character bitmaps (placeholder)
    memory_set(atlas_data, 128, atlas_width * atlas_height); // Gray background
    
    g_font_texture_atlas = graphics_create_texture_2d(atlas_width, atlas_height, atlas_data);
    
    // Initialize glyph cache with placeholder data
    for (int i = 0; i < 256; i++) {
        g_glyph_cache[i].texture_id = g_font_texture_atlas;
        g_glyph_cache[i].advance_x = 12.0f;
        g_glyph_cache[i].advance_y = 16.0f;
        g_glyph_cache[i].bitmap_width = 10.0f;
        g_glyph_cache[i].bitmap_height = 14.0f;
        g_glyph_cache[i].bitmap_left = 1.0f;
        g_glyph_cache[i].bitmap_top = 12.0f;
    }
    
    memory_free(atlas_data);
    printf("RaeenUI: Font system initialized\n");
}

/**
 * Render a rectangle (background, border)
 */
static void raeenui_render_rect(RaeenUIView* view, GraphicsContext* gfx) {
    graphics_use_shader_program(g_rect_shader);
    graphics_bind_vertex_array(g_quad_vao);
    
    // Set uniforms
    graphics_set_uniform_vec4(g_rect_shader, "color", 
        view->style.background_color.r, 
        view->style.background_color.g, 
        view->style.background_color.b, 
        view->style.background_color.a);
    
    graphics_set_uniform_vec2(g_rect_shader, "size", 
        view->frame.size.width, view->frame.size.height);
    
    graphics_set_uniform_float(g_rect_shader, "cornerRadius", view->style.corner_radius);
    
    // Draw
    graphics_draw_indexed(6);
}

/**
 * Render text
 */
static void raeenui_render_text(RaeenUIView* view, GraphicsContext* gfx) {
    if (!view->text_content) return;
    
    graphics_use_shader_program(g_text_shader);
    graphics_bind_vertex_array(g_quad_vao);
    graphics_bind_texture(g_font_texture_atlas);
    
    graphics_set_uniform_vec4(g_text_shader, "textColor",
        view->style.foreground_color.r,
        view->style.foreground_color.g,
        view->style.foreground_color.b,
        view->style.foreground_color.a);
    
    // Render each character
    float x_offset = view->style.padding.left;
    float y_offset = view->style.padding.top;
    
    for (int i = 0; view->text_content[i] != '\0'; i++) {
        char c = view->text_content[i];
        if (c < 0 || c >= 256) continue;
        
        GlyphInfo* glyph = &g_glyph_cache[(int)c];
        
        graphics_set_uniform_vec2(g_text_shader, "glyphPos", x_offset, y_offset);
        graphics_set_uniform_vec2(g_text_shader, "glyphSize", 
            glyph->bitmap_width, glyph->bitmap_height);
        
        graphics_draw_indexed(6);
        
        x_offset += glyph->advance_x;
    }
}

/**
 * Render image
 */
static void raeenui_render_image(RaeenUIView* view, GraphicsContext* gfx) {
    if (!view->image_data || view->texture_id == 0) return;
    
    graphics_use_shader_program(g_rect_shader);
    graphics_bind_vertex_array(g_quad_vao);
    graphics_bind_texture(view->texture_id);
    
    graphics_set_uniform_vec4(g_rect_shader, "color", 1.0f, 1.0f, 1.0f, 1.0f);
    graphics_set_uniform_vec2(g_rect_shader, "size", 
        view->frame.size.width, view->frame.size.height);
    graphics_set_uniform_float(g_rect_shader, "cornerRadius", view->style.corner_radius);
    
    graphics_draw_indexed(6);
}

/**
 * Render button
 */
static void raeenui_render_button(RaeenUIView* view, GraphicsContext* gfx) {
    // Render button background
    RaeenUIColor button_color = view->style.background_color;
    
    // Adjust color based on state
    if (view->is_pressed) {
        button_color.r *= 0.8f;
        button_color.g *= 0.8f;
        button_color.b *= 0.8f;
    } else if (view->is_hovered) {
        button_color.r *= 1.1f;
        button_color.g *= 1.1f;
        button_color.b *= 1.1f;
    }
    
    // Temporarily modify style for rendering
    RaeenUIColor original_color = view->style.background_color;
    view->style.background_color = button_color;
    
    raeenui_render_rect(view, gfx);
    
    // Restore original color
    view->style.background_color = original_color;
    
    // Render button text
    if (view->text_content) {
        raeenui_render_text(view, gfx);
    }
}

/**
 * Apply visual effects
 */
static void raeenui_apply_effects(RaeenUIView* view, GraphicsContext* gfx) {
    // Apply blur effect
    if (view->style.blur_radius > 0.0f) {
        raeenui_render_blur(view, gfx);
    }
}

/**
 * Render shadow effect
 */
static void raeenui_render_shadow(RaeenUIView* view, GraphicsContext* gfx) {
    // Render shadow as a slightly offset, blurred rectangle
    graphics_use_shader_program(g_rect_shader);
    graphics_bind_vertex_array(g_quad_vao);
    
    // Offset shadow position
    float shadow_transform[16];
    graphics_matrix_identity(shadow_transform);
    graphics_matrix_translate(shadow_transform, 
        view->frame.origin.x + view->style.shadow_offset_x,
        view->frame.origin.y + view->style.shadow_offset_y, 
        -0.1f); // Slightly behind
    graphics_set_transform_matrix(gfx, shadow_transform);
    
    // Set shadow color
    graphics_set_uniform_vec4(g_rect_shader, "color",
        view->style.shadow_color.r,
        view->style.shadow_color.g,
        view->style.shadow_color.b,
        view->style.shadow_color.a);
    
    graphics_set_uniform_vec2(g_rect_shader, "size",
        view->frame.size.width, view->frame.size.height);
    graphics_set_uniform_float(g_rect_shader, "cornerRadius", view->style.corner_radius);
    
    graphics_draw_indexed(6);
}

/**
 * Render blur effect
 */
static void raeenui_render_blur(RaeenUIView* view, GraphicsContext* gfx) {
    // Blur implementation would require render-to-texture
    // This is a placeholder for the blur effect
    // In a full implementation, this would:
    // 1. Render view to offscreen texture
    // 2. Apply horizontal blur pass
    // 3. Apply vertical blur pass
    // 4. Composite result back to main framebuffer
}

/**
 * Math helper functions
 */
static float fmaxf(float a, float b) {
    return (a > b) ? a : b;
}

static float fminf(float a, float b) {
    return (a < b) ? a : b;
}
