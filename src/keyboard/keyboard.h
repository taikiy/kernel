#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "idt.h"
#include "status.h"
#include <stdint.h>

#define KEYBOARD_BUFFER_SIZE 1024

typedef status_t (*KB_INIT_CALLBACK)(void);

struct keyboard
{
    char name[32];

    KB_INIT_CALLBACK initialize;
    INTERRUPT_HANDLER_CALLBACK interrupt_handler;
};

struct keyboard_buffer
{
    char buffer[KEYBOARD_BUFFER_SIZE];
    int head;
    int tail;
};

void initialize_keyboard_drivers();
void* keyboard_interrupt_handler(struct interrupt_frame* frame);
void push_key(uint8_t key);

#endif
