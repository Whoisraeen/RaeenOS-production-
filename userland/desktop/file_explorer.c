#include "file_explorer.h"
#include "desktop.h"
#include "../../kernel/graphics.h"
#include "../../kernel/vga.h"
#include "../../kernel/fs/vfs.h"
#include "../../libs/libc/include/string.h"
#include "../../libs/libc/include/string.h"

static bool is_open = false;
static char current_path[256] = "/";

void file_explorer_init(void) {
    debug_print("File Explorer initialized (placeholder).\n");
}

void file_explorer_open(const char* path) {
    debug_print("File Explorer: Opening path ");
    debug_print(path);
    debug_print("\n");
    strncpy(current_path, path, sizeof(current_path) - 1);
    is_open = true;
    // TODO: Read directory contents using VFS
}

void file_explorer_draw(void) {
    if (!is_open) return;

    // Draw a simple window for the file explorer
    uint32_t win_x = 50;
    uint32_t win_y = 50;
    uint32_t win_width = 600;
    uint32_t win_height = 400;

    desktop_draw_window(win_x, win_y, win_width, win_height, 0xCCCCCC, "File Explorer"); // Light grey background
    graphics_draw_string(win_x + 10, win_y + 30, "Current Path: ", 0x000000);
    graphics_draw_string(win_x + 120, win_y + 30, current_path, 0x000000);

    // TODO: List files and folders
    graphics_draw_string(win_x + 10, win_y + 60, "(Files and folders listed here)", 0x000000);

    debug_print("File Explorer: Drawn.\n");
}

void file_explorer_handle_event(uint32_t x, uint32_t y, uint8_t button) {
    if (!is_open) return;

    debug_print("File Explorer: Event at (");
    vga_put_dec(x);
    debug_print(", ");
    vga_put_dec(y);
    debug_print(") button: ");
    vga_put_dec(button);
    debug_print(" (simulated).\n");
    // In a real file explorer, this would handle navigation, opening files, etc.
}
