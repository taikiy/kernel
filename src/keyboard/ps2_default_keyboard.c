#include "ps2_default_keyboard.h"
#include "io/io.h"
#include "terminal/terminal.h"

static uint8_t keyboard_scancode_set_one[] = {
    0x00, 0x1B, '1', '2',  '3', '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=',  0x08, '\t', 'Q',
    'W',  'E',  'R', 'T',  'Y', 'U',  'I',  'O',  'P',  '[',  ']',  0x0d, 0x00, 'A',  'S',  'D',  'F',
    'G',  'H',  'J', 'K',  'L', ';',  '\'', '`',  0x00, '\\', 'Z',  'X',  'C',  'V',  'B',  'N',  'M',
    ',',  '.',  '/', 0x00, '*', 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, '7',  '8', '9',  '-', '4',  '5',  '6',  '+',  '1',  '2',  '3',  '0',  '.',
};

static status_t
initialize_keyboard()
{
    print("Initializing PS/2 Default Keyboard\n");

    outb(PS2_COMMAND_PORT, PS2_COMMAND_ENABLE_FIRST_PORT);

    return ALL_OK;
}

static uint8_t
scancode_to_ascii_char(uint8_t scancode)
{
    if (scancode > (sizeof(keyboard_scancode_set_one) / sizeof(keyboard_scancode_set_one[0]))) {
        return 0;
    }

    char c = keyboard_scancode_set_one[scancode];

    // TODO: handle shift, caps lock, etc.

    return c;
}

struct keyboard*
ps2_default_keyboard()
{
    static struct keyboard keyboard = {
        .name = "PS/2 Default Keyboard",
        .initialize = initialize_keyboard,
    };

    return &keyboard;
}
