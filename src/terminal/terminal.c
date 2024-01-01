#include "terminal.h"
#include "string/string.h"
#include <stddef.h>
#include <stdint.h>

uint16_t* video_mem = 0;
uint16_t terminal_row = 0;
uint16_t terminal_col = 0;

uint16_t
terminal_make_char(char c, char color)
{
    return (color << 8) | c;
}

void
terminal_put_char(int x, int y, char c, char color)
{
    video_mem[y * VGA_WIDTH + x] = terminal_make_char(c, color);
}

void
terminal_write_char(char c, char color)
{
    if (c == '\n') {
        terminal_col = 0;
        terminal_row += 1;
        return;
    }

    terminal_put_char(terminal_col, terminal_row, c, color);
    terminal_col += 1;
    if (terminal_col >= VGA_WIDTH) {
        terminal_col = 0;
        terminal_row += 1;
    }
}

void
terminal_initialize()
{
    video_mem = (uint16_t*)0xB8000;
    terminal_row = 0;
    terminal_col = 0;

    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            terminal_put_char(x, y, ' ', 0);
        }
    }
}

void
print(const char* str)
{
    size_t len = strlen(str);
    for (int i = 0; i < len; i++) {
        terminal_write_char(str[i], 15);
    }
}

void
print_int(int n)
{
    char buf[32];
    itoa(n, buf, 10);
    print(buf);
}
