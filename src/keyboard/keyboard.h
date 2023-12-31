#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "status.h"
#include <stdint.h>

#define KEYBOARD_BUFFER_SIZE 1024

typedef status_t (*KB_INIT_CALLBACK)(void);
typedef void (*KB_KEY_PUSH_CALLBACK)(uint8_t key);
typedef uint8_t (*KB_KEY_POP_CALLBACK)(void);

struct keyboard
{
    char name[16];

    KB_INIT_CALLBACK initialize;
};

struct keyboard_buffer
{
    char buffer[KEYBOARD_BUFFER_SIZE];
    int head;
    int tail;
};

void initialize_keyboard_drivers();

#endif
