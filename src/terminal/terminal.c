#include "terminal.h"
#include "../string/string.h"
#include <stddef.h>
#include <stdint.h>

// Absolute position of the cursor on the screen.
uint16_t cursor_row = 0;
uint16_t cursor_col = 0;

// Protected Mode VGA text mode video memory address.
uint16_t* video_mem = 0;

// Screen buffer is twice the size of the video height to support scrolling.
#define SCREEN_BUFFER_HEIGHT (VGA_HEIGHT * 2)
// Screen buffer to save two bytes per character (uint16_t): one for the character and one for the color.
static uint16_t screen_buffer[VGA_WIDTH * SCREEN_BUFFER_HEIGHT];

static uint16_t
text_mode_character(char c, char color)
{
    // Text mode memory takes two bytes for every "character" on screen. One is the ASCII code byte, the other the
    // attribute byte.
    return (color << 8) | c;
}

static void
push_char_to_buffer(int x, int y, char c, char color)
{
    screen_buffer[(y % SCREEN_BUFFER_HEIGHT) * VGA_WIDTH + x] = text_mode_character(c, color);
}

static void
write_char(char c, char color)
{
    if (c == '\n' || c == '\r') {
        while (cursor_col < VGA_WIDTH) {
            push_char_to_buffer(cursor_col, cursor_row, ' ', color);
            cursor_col += 1;
        }
    } else if (c == 0x08) {
        // Backspace
        if (cursor_col == 0 && cursor_row == 0) {
            return;
        } else if (cursor_col == 0) {
            cursor_col = VGA_WIDTH - 1;
            cursor_row -= 1;
        } else {
            cursor_col -= 1;
        }
        push_char_to_buffer(cursor_col, cursor_row, ' ', color);
    } else {
        push_char_to_buffer(cursor_col, cursor_row, c, color);
        cursor_col += 1;
    }

    if (cursor_col >= VGA_WIDTH) {
        cursor_col = 0;
        cursor_row += 1;
    }
}

static int
get_visible_screen_top_row()
{
    // TODO: This doesn't quite work. We are not accounting for the fact that the cursor can be moved up and down

    int top = (cursor_row - VGA_HEIGHT) % SCREEN_BUFFER_HEIGHT;
    return top < 0 ? 0 : top;
}

static void
flush_screen_buffer()
{
    int top_row = get_visible_screen_top_row();
    for (int y = 0; y < VGA_HEIGHT; y++) {
        int buffer_y = (y + top_row) % SCREEN_BUFFER_HEIGHT;
        for (int x = 0; x < VGA_WIDTH; x++) {
            video_mem[y * VGA_WIDTH + x] = screen_buffer[buffer_y * VGA_WIDTH + x];
        }
    }
}

void
terminal_initialize()
{
    // TODO: https://wiki.osdev.org/Detecting_Colour_and_Monochrome_Monitors

    video_mem = (uint16_t*)0xB8000;
    cursor_col = 0;
    cursor_row = 0;

    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            push_char_to_buffer(x, y, ' ', 0);
        }
    }

    flush_screen_buffer();
}

void
print(const char* str)
{
    size_t len = strlen(str);
    for (int i = 0; i < len; i++) {
        write_char(str[i], 15);
    }
    flush_screen_buffer();
}

void
print_int(int n)
{
    char buf[32];
    itoa(n, buf, 10);
    print(buf);
}

void
print_hex(uint32_t n)
{
    char buf[32];
    itoa(n, buf, 16);
    print(buf);
}
