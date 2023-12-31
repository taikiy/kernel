#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "status.h"
#include <stdint.h>

#define KEYBOARD_BUFFER_SIZE 1024

typedef status_t (*KB_INIT_CALLBACK)(void);

struct keyboard
{
    char name[32];

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
