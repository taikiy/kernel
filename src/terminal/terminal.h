#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>

#define VGA_WIDTH  80
#define VGA_HEIGHT 25

enum COLORS
{
    BLACK = 0,
    BLUE,
    GREEN,
    CYAN,
    RED,
    PURPLE,
    BROWN,
    GRAY,
    DARK_GRAY,
    LIGHT_BLUE,
    LIGHT_GREEN,
    LIGHT_CYAN,
    LIGHT_RED,
    LIGHT_PURPLE,
    YELLOW,
    WHITE,
};

void terminal_initialize();
void print(const char* str);
void printc(const char* str, enum COLORS color);
void print_int(int n);
void print_hex(uint32_t n);

#endif
