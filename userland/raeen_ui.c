#include "raeen_ui.h"
#include "../kernel/vga.h"

void raeen_ui_init(void) {
    vga_puts("RaeenOS UI framework initialized (placeholder).\n");
}

raeen_ui_element_t* raeen_ui_create_element(raeen_ui_element_type_t type, int x, int y, int width, int height, const char* text) {
    (void)type;
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)text;
    vga_puts("Creating RaeenOS UI element (placeholder).\n");
    return NULL; // Not implemented
}

void raeen_ui_draw_element(raeen_ui_element_t* element) {
    (void)element;
    vga_puts("Drawing RaeenOS UI element (placeholder).\n");
}

