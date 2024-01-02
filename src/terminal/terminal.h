#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>

#define VGA_WIDTH  80
#define VGA_HEIGHT 25

void terminal_initialize();
void print(const char* str);
void print_int(int n);
void print_hex(uint32_t n);

#endif
