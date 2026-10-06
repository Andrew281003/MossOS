#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include "multiboot.h"

// Colors in RGB (we'll assume 32-bit framebuffer layout: ARGB or BGRA)
// Standard Win95 Colors
#define COLOR_TEAL      0x00008080
#define COLOR_SILVER    0x00C0C0C0
#define COLOR_GRAY      0x00808080
#define COLOR_WHITE     0x00FFFFFF
#define COLOR_BLACK     0x00000000
#define COLOR_BLUE      0x00000080
#define COLOR_DARK_BLUE 0x000000A0
#define COLOR_RED       0x00FF0000

void init_graphics(struct multiboot_info* mbd);
void put_pixel(int x, int y, uint32_t color);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void draw_char(char c, int x, int y, uint32_t color, uint32_t bg_color);
void draw_string(const char* str, int x, int y, uint32_t color, uint32_t bg_color);
void draw_win95_desktop(int mx, int my, int mb);
void swap_buffers();
void draw_mouse_cursor(int x, int y);

#endif
