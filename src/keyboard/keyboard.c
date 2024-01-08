#include "keyboard.h"
#include "../config.h"
#include "../memory/memory.h"
#include "../system/sys.h"
#include "../task/process.h"
#include "../task/task.h"
#include "../terminal/terminal.h"
#include "ps2_default_keyboard.h"

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
    if (register_keyboard_driver(ps2_default_keyboard_driver()) != ALL_OK) {
        panic("Failed to register PS/2 keyboard driver\n");
    }
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

/// @brief This function is called from the interrupt handler in src/idt/idt.c. The Interrupt handler calls
/// `switch_to_kernel_page()` before calling this function.
/// @param frame The interrupt frame that contains the current task registers.
/// @return This interrupt handler always returns 0. Pressed keys are pushed to the keyboard buffer.
void*
keyboard_interrupt_handler(struct interrupt_frame* frame)
{
    struct keyboard* keyboard = current_keyboard();
    if (!keyboard) {
        return 0;
    }

    char c = (uint32_t)keyboard->interrupt_handler(frame);
    if (c != 0) {
        push_key(c);
    }

    return 0;
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

uint8_t
pop_key()
{
    struct task* current_task = get_current_task();
    if (!current_task) {
        return 0;
    }

    struct process* current_process = current_task->process;
    if (!current_process) {
        return 0;
    }

    int head = current_process->keyboard_buffer.head % KEYBOARD_BUFFER_SIZE;
    int tail = current_process->keyboard_buffer.tail % KEYBOARD_BUFFER_SIZE;

    if (head == tail) {
        // Buffer is empty
        return 0;
    }

    uint8_t key = current_process->keyboard_buffer.buffer[head];
    current_process->keyboard_buffer.head++;

    return key;
}
