#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../idt/idt.h"
#include "../status.h"
#include <stdbool.h>
#include <stdint.h>

#define KEYBOARD_BUFFER_SIZE 1024

#define BACKSPACE 0x08
#define CTRL_C    0x03
#define CTRL_D    0x04

typedef status_t (*KB_INIT_CALLBACK)(void);

struct keyboard
{
    char name[32];

    KB_INIT_CALLBACK initialize;
    INTERRUPT_HANDLER interrupt_handler;

    bool is_shift_pressed;
    bool is_ctrl_pressed;
};

struct keyboard_buffer
{
    char buffer[KEYBOARD_BUFFER_SIZE];
    int head;
    int tail;
};

void initialize_keyboard_drivers();
void* keyboard_interrupt_handler(struct interrupt_frame* frame);
struct keyboard* current_keyboard();
void push_key(uint8_t key);
uint8_t pop_key();

#endif
