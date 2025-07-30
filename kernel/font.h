// RaeenOS Font Engine Interface

#ifndef FONT_H
#define FONT_H

#include <stdint.h>

#define FONT_WIDTH 8
#define FONT_HEIGHT 8

/**
 * @brief Provides access to the bitmap data for a single character.
 * 
 * @param c The ASCII character.
 * @return const uint8_t* A pointer to the 8-byte bitmap for the character, or NULL if not printable.
 */
const uint8_t* font_get_char(char c);

#endif // FONT_H
