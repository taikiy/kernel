#include "keyboard.h"
#include "config.h"
#include "memory/memory.h"
#include "task/process.h"

// We could have used a linked list here. Not sure if that's a better design though.
static struct keyboard* keyboard_drivers[MAX_KEYBOARD_DRIVER_COUNT];
static int current_keyboard_index;

static struct keyboard*
current_keyboard()
{
    return keyboard_drivers[current_keyboard_index % MAX_KEYBOARD_DRIVER_COUNT];
}

static struct keyboard**
get_free_keyboard_driver_slot()
{
    for (int i = 0; i < MAX_KEYBOARD_DRIVER_COUNT; i++) {
        if (keyboard_drivers[i] == 0) {
            return &keyboard_drivers[i];
        }
    }
    return 0;
}

static status_t
register_keyboard_driver(struct keyboard* driver)
{
    if (!driver->initialize) {
        return ERROR(ENOCALLBACK);
    }

    struct keyboard** free_slot = get_free_keyboard_driver_slot();
    if (free_slot == 0) {
        return ERROR(ETOOMANYDRIVERS);
    }
    *free_slot = driver;

    return driver->initialize();
}

static status_t
load_static_keyboard_drivers()
{
    // It's okay to panic here because these are static drivers.
    // if (register_keyboard_driver(ps2_keyboard_driver()) != ALL_OK) {
    //     panic("Failed to register PS/2 keyboard driver\n");
    // }
    return ALL_OK;
}

static status_t
load_keyboard_drivers()
{
    return load_static_keyboard_drivers();
}

void
initialize_keyboard_drivers()
{
    memset(keyboard_drivers, 0, sizeof(keyboard_drivers));

    load_keyboard_drivers();

    current_keyboard_index = 0;
}

void
push_key(uint8_t key)
{
    struct process* current_process = get_current_process();
    if (!current_process) {
        return;
    }

    int head = current_process->keyboard_buffer.head % KEYBOARD_BUFFER_SIZE;
    int tail = current_process->keyboard_buffer.tail % KEYBOARD_BUFFER_SIZE;

    if (tail + 1 == head) {
        // Buffer is full
        return;
    }

    current_process->keyboard_buffer.buffer[tail] = key;
    current_process->keyboard_buffer.tail++;
}
