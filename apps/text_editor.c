/**
 * RaeenOS Text Editor Application
 * Modern code editor with syntax highlighting and RaeenUI integration
 */

#include "../ui/raeenui.h"
#include "../ui/desktop_shell.h"
#include "../kernel/fs/fat32/fat32_production.h"
#include "../kernel/memory.h"
#include "string.h"

// Text Buffer Structure
typedef struct text_line {
    char* content;
    uint32_t length;
    uint32_t capacity;
    bool modified;
    struct text_line* next;
    struct text_line* prev;
} text_line_t;

// Cursor Position
typedef struct {
    uint32_t line;
    uint32_t column;
} cursor_pos_t;

// Selection Range
typedef struct {
    cursor_pos_t start;
    cursor_pos_t end;
    bool active;
} selection_t;

// Syntax Highlighting
typedef enum {
    TOKEN_NORMAL,
    TOKEN_KEYWORD,
    TOKEN_STRING,
    TOKEN_COMMENT,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_IDENTIFIER
} token_type_t;

typedef struct {
    token_type_t type;
    uint32_t start;
    uint32_t length;
    RaeenUIColor color;
} syntax_token_t;

// Text Editor State
typedef struct {
    RaeenUIWindow* window;
    RaeenUIView* menu_bar;
    RaeenUIView* toolbar;
    RaeenUIView* editor_area;
    RaeenUIView* status_bar;
    RaeenUIView* line_numbers;
    RaeenUIView* text_view;
    
    // Document state
    text_line_t* lines;
    uint32_t line_count;
    cursor_pos_t cursor;
    selection_t selection;
    
    // File information
    char file_path[512];
    bool modified;
    bool read_only;
    
    // Editor settings
    uint32_t tab_size;
    bool show_line_numbers;
    bool word_wrap;
    bool syntax_highlighting;
    char* language;
    
    // Undo/Redo
    struct editor_action* undo_stack;
    struct editor_action* redo_stack;
    uint32_t undo_count;
    uint32_t redo_count;
    
    // Search/Replace
    char search_term[256];
    char replace_term[256];
    bool search_case_sensitive;
    bool search_whole_word;
} text_editor_t;

// Editor Action for Undo/Redo
typedef struct editor_action {
    enum {
        ACTION_INSERT,
        ACTION_DELETE,
        ACTION_REPLACE
    } type;
    cursor_pos_t position;
    char* text;
    uint32_t length;
    struct editor_action* next;
} editor_action_t;

static text_editor_t g_text_editor = {0};

// Function declarations
static bool text_editor_init(void);
static void text_editor_create_ui(void);
static void text_editor_create_menu_bar(void);
static void text_editor_create_toolbar(void);
static void text_editor_create_editor_area(void);
static void text_editor_create_status_bar(void);
static bool text_editor_load_file(const char* file_path);
static bool text_editor_save_file(const char* file_path);
static void text_editor_insert_text(const char* text);
static void text_editor_delete_text(uint32_t length);
static void text_editor_move_cursor(int32_t line_delta, int32_t col_delta);
static void text_editor_set_cursor(uint32_t line, uint32_t column);
static void text_editor_update_display(void);
static void text_editor_apply_syntax_highlighting(text_line_t* line);
static void text_editor_handle_key_input(uint32_t key_code, uint32_t modifiers);
static text_line_t* text_editor_get_line(uint32_t line_number);
static void text_editor_add_undo_action(editor_action_t* action);

// Event handlers
static void text_editor_handle_new_file(RaeenUIView* view, void* user_data);
static void text_editor_handle_open_file(RaeenUIView* view, void* user_data);
static void text_editor_handle_save_file(RaeenUIView* view, void* user_data);
static void text_editor_handle_save_as(RaeenUIView* view, void* user_data);
static void text_editor_handle_undo(RaeenUIView* view, void* user_data);
static void text_editor_handle_redo(RaeenUIView* view, void* user_data);
static void text_editor_handle_cut(RaeenUIView* view, void* user_data);
static void text_editor_handle_copy(RaeenUIView* view, void* user_data);
static void text_editor_handle_paste(RaeenUIView* view, void* user_data);
static void text_editor_handle_find(RaeenUIView* view, void* user_data);
static void text_editor_handle_replace(RaeenUIView* view, void* user_data);

/**
 * Launch text editor application
 */
int text_editor_main(int argc, char* argv[]) {
    printf("Text Editor: Starting application...\n");
    
    if (!text_editor_init()) {
        printf("Text Editor: Failed to initialize\n");
        return 1;
    }
    
    text_editor_create_ui();
    
    // Load file if specified
    if (argc > 1) {
        text_editor_load_file(argv[1]);
    } else {
        // Create new document
        text_editor_handle_new_file(NULL, NULL);
    }
    
    printf("Text Editor: Application started successfully\n");
    return 0;
}

/**
 * Initialize text editor
 */
static bool text_editor_init(void) {
    text_editor_t* editor = &g_text_editor;
    
    memory_set(editor, 0, sizeof(text_editor_t));
    
    // Default settings
    editor->tab_size = 4;
    editor->show_line_numbers = true;
    editor->word_wrap = false;
    editor->syntax_highlighting = true;
    editor->language = string_duplicate("text");
    
    // Initialize cursor
    editor->cursor.line = 0;
    editor->cursor.column = 0;
    
    return true;
}

/**
 * Create text editor UI
 */
static void text_editor_create_ui(void) {
    text_editor_t* editor = &g_text_editor;
    
    // Create main window
    editor->window = desktop_shell_create_application_window("Text Editor", 2, 900, 700);
    if (!editor->window) {
        printf("Text Editor: Failed to create window\n");
        return;
    }
    
    // Create main layout
    RaeenUIView* main_layout = RaeenUIVStack(0);
    
    // Create components
    text_editor_create_menu_bar();
    text_editor_create_toolbar();
    text_editor_create_editor_area();
    text_editor_create_status_bar();
    
    // Assemble layout
    raeenui_view_add_child(main_layout, editor->menu_bar);
    raeenui_view_add_child(main_layout, editor->toolbar);
    raeenui_view_add_child(main_layout, editor->editor_area);
    raeenui_view_add_child(main_layout, editor->status_bar);
    
    raeenui_window_set_content_view(editor->window, main_layout);
}

/**
 * Create menu bar
 */
static void text_editor_create_menu_bar(void) {
    text_editor_t* editor = &g_text_editor;
    
    editor->menu_bar = RaeenUIHStack(0);
    raeenui_view_set_size(editor->menu_bar, (RaeenUISize){0, 28});
    raeenui_background(editor->menu_bar, raeenui_color_rgba(0.96f, 0.96f, 0.96f, 1.0f));
    
    // File menu
    RaeenUIView* file_menu = RaeenUIButton("File", NULL);
    RaeenUIView* edit_menu = RaeenUIButton("Edit", NULL);
    RaeenUIView* view_menu = RaeenUIButton("View", NULL);
    RaeenUIView* tools_menu = RaeenUIButton("Tools", NULL);
    RaeenUIView* help_menu = RaeenUIButton("Help", NULL);
    
    raeenui_font_size(file_menu, 12);
    raeenui_font_size(edit_menu, 12);
    raeenui_font_size(view_menu, 12);
    raeenui_font_size(tools_menu, 12);
    raeenui_font_size(help_menu, 12);
    
    raeenui_padding(file_menu, 8);
    raeenui_padding(edit_menu, 8);
    raeenui_padding(view_menu, 8);
    raeenui_padding(tools_menu, 8);
    raeenui_padding(help_menu, 8);
    
    raeenui_view_add_child(editor->menu_bar, file_menu);
    raeenui_view_add_child(editor->menu_bar, edit_menu);
    raeenui_view_add_child(editor->menu_bar, view_menu);
    raeenui_view_add_child(editor->menu_bar, tools_menu);
    raeenui_view_add_child(editor->menu_bar, help_menu);
}

/**
 * Create toolbar
 */
static void text_editor_create_toolbar(void) {
    text_editor_t* editor = &g_text_editor;
    
    editor->toolbar = RaeenUIHStack(4);
    raeenui_view_set_size(editor->toolbar, (RaeenUISize){0, 36});
    raeenui_background(editor->toolbar, raeenui_color_rgba(0.94f, 0.94f, 0.94f, 1.0f));
    raeenui_padding(editor->toolbar, 6);
    
    // File operations
    RaeenUIView* new_btn = RaeenUIButton("📄", text_editor_handle_new_file);
    RaeenUIView* open_btn = RaeenUIButton("📁", text_editor_handle_open_file);
    RaeenUIView* save_btn = RaeenUIButton("💾", text_editor_handle_save_file);
    
    // Edit operations
    RaeenUIView* undo_btn = RaeenUIButton("↶", text_editor_handle_undo);
    RaeenUIView* redo_btn = RaeenUIButton("↷", text_editor_handle_redo);
    
    // Clipboard operations
    RaeenUIView* cut_btn = RaeenUIButton("✂️", text_editor_handle_cut);
    RaeenUIView* copy_btn = RaeenUIButton("📋", text_editor_handle_copy);
    RaeenUIView* paste_btn = RaeenUIButton("📄", text_editor_handle_paste);
    
    // Search operations
    RaeenUIView* find_btn = RaeenUIButton("🔍", text_editor_handle_find);
    RaeenUIView* replace_btn = RaeenUIButton("🔄", text_editor_handle_replace);
    
    // Set button sizes
    RaeenUIView* buttons[] = {new_btn, open_btn, save_btn, undo_btn, redo_btn, 
                             cut_btn, copy_btn, paste_btn, find_btn, replace_btn};
    
    for (int i = 0; i < 10; i++) {
        raeenui_view_set_size(buttons[i], (RaeenUISize){28, 24});
        raeenui_corner_radius(buttons[i], 4);
        raeenui_view_add_child(editor->toolbar, buttons[i]);
    }
    
    // Add separator
    RaeenUIView* separator = RaeenUIView();
    raeenui_view_set_size(separator, (RaeenUISize){1, 20});
    raeenui_background(separator, raeenui_color_rgba(0.7f, 0.7f, 0.7f, 1.0f));
    raeenui_view_add_child(editor->toolbar, separator);
    
    // Language selector
    RaeenUIView* lang_label = RaeenUIText("Language:");
    raeenui_font_size(lang_label, 11);
    RaeenUIView* lang_selector = RaeenUIButton("Text", NULL);
    raeenui_font_size(lang_selector, 11);
    
    raeenui_view_add_child(editor->toolbar, lang_label);
    raeenui_view_add_child(editor->toolbar, lang_selector);
}

/**
 * Create editor area
 */
static void text_editor_create_editor_area(void) {
    text_editor_t* editor = &g_text_editor;
    
    editor->editor_area = RaeenUIHStack(0);
    raeenui_view_set_flex_grow(editor->editor_area, 1.0f);
    raeenui_background(editor->editor_area, raeenui_color_white());
    
    // Line numbers
    if (editor->show_line_numbers) {
        editor->line_numbers = RaeenUIVStack(0);
        raeenui_view_set_size(editor->line_numbers, (RaeenUISize){50, 0});
        raeenui_background(editor->line_numbers, raeenui_color_rgba(0.98f, 0.98f, 0.98f, 1.0f));
        raeenui_padding(editor->line_numbers, 4);
        
        // Add line numbers (1-50 for demo)
        for (int i = 1; i <= 50; i++) {
            char line_num[8];
            string_format(line_num, sizeof(line_num), "%d", i);
            RaeenUIView* num_label = RaeenUIText(line_num);
            raeenui_font_size(num_label, 11);
            raeenui_font_family(num_label, "monospace");
            raeenui_foreground(num_label, raeenui_color_rgba(0.5f, 0.5f, 0.5f, 1.0f));
            raeenui_view_add_child(editor->line_numbers, num_label);
        }
        
        raeenui_view_add_child(editor->editor_area, editor->line_numbers);
    }
    
    // Text editing area
    editor->text_view = RaeenUIScrollView();
    raeenui_view_set_flex_grow(editor->text_view, 1.0f);
    raeenui_background(editor->text_view, raeenui_color_white());
    
    // Create text content area
    RaeenUIView* text_content = RaeenUIVStack(0);
    raeenui_padding(text_content, 8);
    
    // Add sample text lines
    const char* sample_lines[] = {
        "// Welcome to RaeenOS Text Editor",
        "#include <stdio.h>",
        "",
        "int main() {",
        "    printf(\"Hello, RaeenOS!\\n\");",
        "    return 0;",
        "}",
        "",
        "/* This is a sample C program */",
        "// Features:",
        "// - Syntax highlighting",
        "// - Line numbers",
        "// - Undo/Redo",
        "// - Search/Replace",
        NULL
    };
    
    for (int i = 0; sample_lines[i] != NULL; i++) {
        RaeenUIView* line = RaeenUIText(sample_lines[i]);
        raeenui_font_family(line, "monospace");
        raeenui_font_size(line, 13);
        raeenui_view_set_size(line, (RaeenUISize){0, 18});
        
        // Apply basic syntax highlighting
        if (string_starts_with(sample_lines[i], "//") || string_starts_with(sample_lines[i], "/*")) {
            raeenui_foreground(line, raeenui_color_rgba(0.0f, 0.6f, 0.0f, 1.0f)); // Green for comments
        } else if (string_contains(sample_lines[i], "#include") || string_contains(sample_lines[i], "int") || 
                  string_contains(sample_lines[i], "return")) {
            raeenui_foreground(line, raeenui_color_rgba(0.0f, 0.0f, 0.8f, 1.0f)); // Blue for keywords
        } else if (string_contains(sample_lines[i], "\"")) {
            raeenui_foreground(line, raeenui_color_rgba(0.8f, 0.0f, 0.0f, 1.0f)); // Red for strings
        }
        
        raeenui_view_add_child(text_content, line);
    }
    
    raeenui_scroll_view_set_content(editor->text_view, text_content);
    raeenui_view_add_child(editor->editor_area, editor->text_view);
}

/**
 * Create status bar
 */
static void text_editor_create_status_bar(void) {
    text_editor_t* editor = &g_text_editor;
    
    editor->status_bar = RaeenUIHStack(8);
    raeenui_view_set_size(editor->status_bar, (RaeenUISize){0, 24});
    raeenui_background(editor->status_bar, raeenui_color_rgba(0.92f, 0.92f, 0.92f, 1.0f));
    raeenui_padding(editor->status_bar, 6);
    
    // File status
    RaeenUIView* file_status = RaeenUIText("Untitled");
    raeenui_font_size(file_status, 11);
    raeenui_view_set_flex_grow(file_status, 1.0f);
    
    // Cursor position
    RaeenUIView* cursor_pos = RaeenUIText("Line 1, Col 1");
    raeenui_font_size(cursor_pos, 11);
    
    // File encoding
    RaeenUIView* encoding = RaeenUIText("UTF-8");
    raeenui_font_size(encoding, 11);
    
    // Line ending
    RaeenUIView* line_ending = RaeenUIText("LF");
    raeenui_font_size(line_ending, 11);
    
    raeenui_view_add_child(editor->status_bar, file_status);
    raeenui_view_add_child(editor->status_bar, cursor_pos);
    raeenui_view_add_child(editor->status_bar, encoding);
    raeenui_view_add_child(editor->status_bar, line_ending);
}

/**
 * Load file into editor
 */
static bool text_editor_load_file(const char* file_path) {
    text_editor_t* editor = &g_text_editor;
    
    if (!file_path) return false;
    
    // Read file content
    char buffer[65536];
    int bytes_read = fat32_read_file(file_path, buffer, 0, sizeof(buffer) - 1);
    
    if (bytes_read < 0) {
        printf("Text Editor: Failed to read file %s\n", file_path);
        return false;
    }
    
    buffer[bytes_read] = '\0';
    
    // Clear existing content
    text_line_t* line = editor->lines;
    while (line) {
        text_line_t* next = line->next;
        if (line->content) {
            memory_free(line->content);
        }
        memory_free(line);
        line = next;
    }
    editor->lines = NULL;
    editor->line_count = 0;
    
    // Parse content into lines
    const char* line_start = buffer;
    const char* ptr = buffer;
    
    while (*ptr) {
        if (*ptr == '\n' || *ptr == '\r') {
            // Create new line
            text_line_t* new_line = (text_line_t*)memory_alloc(sizeof(text_line_t));
            if (!new_line) break;
            
            memory_set(new_line, 0, sizeof(text_line_t));
            
            uint32_t line_length = ptr - line_start;
            new_line->capacity = line_length + 64; // Extra space for editing
            new_line->content = (char*)memory_alloc(new_line->capacity);
            memory_copy(new_line->content, line_start, line_length);
            new_line->content[line_length] = '\0';
            new_line->length = line_length;
            
            // Add to list
            if (!editor->lines) {
                editor->lines = new_line;
            } else {
                text_line_t* last = editor->lines;
                while (last->next) last = last->next;
                last->next = new_line;
                new_line->prev = last;
            }
            editor->line_count++;
            
            // Skip line ending characters
            if (*ptr == '\r' && *(ptr + 1) == '\n') ptr++;
            ptr++;
            line_start = ptr;
        } else {
            ptr++;
        }
    }
    
    // Handle last line if no trailing newline
    if (line_start < ptr) {
        text_line_t* new_line = (text_line_t*)memory_alloc(sizeof(text_line_t));
        if (new_line) {
            memory_set(new_line, 0, sizeof(text_line_t));
            uint32_t line_length = ptr - line_start;
            new_line->capacity = line_length + 64;
            new_line->content = (char*)memory_alloc(new_line->capacity);
            memory_copy(new_line->content, line_start, line_length);
            new_line->content[line_length] = '\0';
            new_line->length = line_length;
            
            if (!editor->lines) {
                editor->lines = new_line;
            } else {
                text_line_t* last = editor->lines;
                while (last->next) last = last->next;
                last->next = new_line;
                new_line->prev = last;
            }
            editor->line_count++;
        }
    }
    
    // Update editor state
    string_copy(editor->file_path, file_path, sizeof(editor->file_path));
    editor->modified = false;
    editor->cursor.line = 0;
    editor->cursor.column = 0;
    
    text_editor_update_display();
    
    printf("Text Editor: Loaded file %s (%u lines)\n", file_path, editor->line_count);
    return true;
}

/**
 * Save file from editor
 */
static bool text_editor_save_file(const char* file_path) {
    text_editor_t* editor = &g_text_editor;
    
    if (!file_path) return false;
    
    // Build content string
    uint32_t total_size = 0;
    text_line_t* line = editor->lines;
    while (line) {
        total_size += line->length + 1; // +1 for newline
        line = line->next;
    }
    
    char* content = (char*)memory_alloc(total_size + 1);
    if (!content) return false;
    
    uint32_t offset = 0;
    line = editor->lines;
    while (line) {
        memory_copy(content + offset, line->content, line->length);
        offset += line->length;
        if (line->next) {
            content[offset++] = '\n';
        }
        line = line->next;
    }
    content[offset] = '\0';
    
    // Write to file
    int bytes_written = fat32_write_file(file_path, content, 0, offset);
    memory_free(content);
    
    if (bytes_written != (int)offset) {
        printf("Text Editor: Failed to save file %s\n", file_path);
        return false;
    }
    
    // Update editor state
    string_copy(editor->file_path, file_path, sizeof(editor->file_path));
    editor->modified = false;
    
    printf("Text Editor: Saved file %s (%u bytes)\n", file_path, offset);
    return true;
}

// Event handlers (simplified implementations)

static void text_editor_handle_new_file(RaeenUIView* view, void* user_data) {
    text_editor_t* editor = &g_text_editor;
    
    // Clear content and create empty document
    editor->file_path[0] = '\0';
    editor->modified = false;
    editor->cursor.line = 0;
    editor->cursor.column = 0;
    
    printf("Text Editor: New file created\n");
}

static void text_editor_handle_open_file(RaeenUIView* view, void* user_data) {
    // TODO: Show file dialog
    text_editor_load_file("/sample.txt");
}

static void text_editor_handle_save_file(RaeenUIView* view, void* user_data) {
    text_editor_t* editor = &g_text_editor;
    
    if (editor->file_path[0] != '\0') {
        text_editor_save_file(editor->file_path);
    } else {
        text_editor_handle_save_as(view, user_data);
    }
}

static void text_editor_handle_save_as(RaeenUIView* view, void* user_data) {
    // TODO: Show save dialog
    text_editor_save_file("/untitled.txt");
}

static void text_editor_handle_undo(RaeenUIView* view, void* user_data) {
    printf("Text Editor: Undo\n");
    // TODO: Implement undo functionality
}

static void text_editor_handle_redo(RaeenUIView* view, void* user_data) {
    printf("Text Editor: Redo\n");
    // TODO: Implement redo functionality
}

static void text_editor_handle_find(RaeenUIView* view, void* user_data) {
    printf("Text Editor: Find\n");
    // TODO: Show find dialog
}

static void text_editor_handle_replace(RaeenUIView* view, void* user_data) {
    printf("Text Editor: Replace\n");
    // TODO: Show replace dialog
}
