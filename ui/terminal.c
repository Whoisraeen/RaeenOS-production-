// RaeenOS Terminal Widget Implementation
// ------------------------------------

#include "terminal.h"
#include "../pmm.h"
#include "../string.h"
#include "../graphics.h"
#include "../font.h"
#include "../ipc/pipe.h"

#define TERM_COLS 80
#define TERM_ROWS 25

// Internal state for a terminal widget
typedef struct {
    char buffer[TERM_ROWS][TERM_COLS];
    int cursor_x;
    int cursor_y;
    uint32_t fg_color;
    uint32_t bg_color;
    pipe_t* pipe_in;  // Data from shell -> terminal
    pipe_t* pipe_out; // Data from terminal -> shell
} terminal_state_t;

// --- Private Functions ---

// Forward declaration for the draw function


// --- Public API Implementation ---

widget_t* terminal_create(struct window_t* parent, int x, int y, int width, int height) {
    // Create the generic widget
    widget_t* terminal_widget = widget_create(parent, WIDGET_TYPE_TERMINAL, x, y, width, height, NULL);
    if (!terminal_widget) return NULL;

    // Allocate and initialize the terminal's specific state
    terminal_state_t* state = (terminal_state_t*)pmm_alloc_frame();
    if (!state) {
        // In a real scenario, we should free the widget here, but we lack that function
        return NULL;
    }

    memset(state->buffer, 0, sizeof(state->buffer));
    state->cursor_x = 0;
    state->cursor_y = 0;
    state->fg_color = 0xFFFFFFFF; // White
    state->bg_color = 0xFF000000; // Black

    // Create pipes for communication
    state->pipe_in = pipe_create();
    state->pipe_out = pipe_create();
    if (!state->pipe_in || !state->pipe_out) {
        if (state->pipe_in) pipe_destroy(state->pipe_in);
        if (state->pipe_out) pipe_destroy(state->pipe_out);
        pmm_free_frame(state);
        // In a real scenario, we should free the widget here
        return NULL;
    }

    // We'll repurpose the 'text' pointer to hold our state.
    // A better design would be a `void* data` field in widget_t.
    terminal_widget->text = (char*)state;

    return terminal_widget;
}

void terminal_handle_keypress(widget_t* terminal, char c) {
    if (!terminal || !terminal->text) return;
    terminal_state_t* state = (terminal_state_t*)terminal->text;

    // Write the character to the output pipe (to be read by the shell)
    if (state->pipe_out) {
        pipe_write(state->pipe_out, (const uint8_t*)&c, 1);
    }
}

void terminal_process_input(widget_t* terminal) {
    if (!terminal || !terminal->text) return;
    terminal_state_t* state = (terminal_state_t*)terminal->text;
    if (!state->pipe_in) return;

    char c;
    while (pipe_read(state->pipe_in, (uint8_t*)&c, 1) > 0) {
        // This is where the old terminal_handle_keypress logic goes
        if (c == '\n') {
            state->cursor_x = 0;
            state->cursor_y++;
        } else if (c == '\b') { // Handle backspace
            if (state->cursor_x > 0) {
                state->cursor_x--;
                state->buffer[state->cursor_y][state->cursor_x] = ' ';
            }
        } else if (c >= ' ') { // Printable characters
            state->buffer[state->cursor_y][state->cursor_x] = c;
            state->cursor_x++;
        }

        // Simple line wrapping
        if (state->cursor_x >= TERM_COLS) {
            state->cursor_x = 0;
            state->cursor_y++;
        }

        // TODO: Implement scrolling when cursor_y >= TERM_ROWS
    }
}

void terminal_write_string(widget_t* terminal, const char* str) {
    if (!terminal || !str) return;
    while (*str) {
        terminal_handle_keypress(terminal, *str++);
    }
}

pipe_t* terminal_get_input_pipe(widget_t* terminal) {
    if (!terminal || !terminal->text) return NULL;
    terminal_state_t* state = (terminal_state_t*)terminal->text;
    return state->pipe_in;
}

pipe_t* terminal_get_output_pipe(widget_t* terminal) {
    if (!terminal || !terminal->text) return NULL;
    terminal_state_t* state = (terminal_state_t*)terminal->text;
    return state->pipe_out;
}

// --- Drawing Logic (to be called by widget_draw) ---

void draw_terminal(widget_t* widget, struct window_t* parent) {
    if (!widget || !widget->text || !parent) return;

    terminal_state_t* state = (terminal_state_t*)widget->text;

    // Draw the background
    window_draw_rect(parent, widget->x, widget->y, widget->width, widget->height, state->bg_color);

    // Draw the text from the buffer
    for (int row = 0; row < TERM_ROWS; ++row) {
        for (int col = 0; col < TERM_COLS; ++col) {
            char c = state->buffer[row][col];
            if (c) {
                window_draw_char(parent, widget->x + col * FONT_WIDTH, widget->y + row * FONT_HEIGHT, c, state->fg_color);
            }
        }
    }

    // Draw the cursor (a simple block cursor)
    window_draw_rect(parent, widget->x + state->cursor_x * FONT_WIDTH, widget->y + state->cursor_y * FONT_HEIGHT, FONT_WIDTH, FONT_HEIGHT, state->fg_color);
}


