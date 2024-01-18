#include "ps2_default_keyboard.h"
#include "../io/io.h"
#include "../terminal/terminal.h"
#include <stdbool.h>

// Define key mapping tables that maps scancode to e.g., US QWERTY ASCII characters.
// For example, if 'A' is pressed, all we know is that the key corresponding to scancode 0x1E is pressed. That key
// is 'A' on a US QWERTY keyboard, but it's 'Q' on a French AZERTY keyboard. We should have a key mapping table that
// maps scancode 0x1E to 'A' for US QWERTY, and 'Q' for French AZERTY.
static uint8_t us_qwerty_ascii_keymap[] = {
    /*
     0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F    */
    0x00, 0x1B, '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=',  0x08, '\t', // 0
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']',  0x0d, 0x00, 'a',  's',  // 1
    'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  '\'', '`',  0x00, '\\', 'z',  'x',  'c',  'v',  // 2
    'b',  'n',  'm',  ',',  '.',  '/',  0x00, '*',  0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 3
    0x00, 0x00, 0x00, 0x00, 0x00, '7',  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',  '2',  '3',  // 4
    '0',  '.',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 5
};
static uint8_t us_qwerty_ascii_keymap_shifted[] = {
    /*
     0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F    */
    0x00, 0x1B, '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',  '_',  '+',  0x08, '\t', // 0
    'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '{',  '}',  0x0d, 0x00, 'A',  'S',  // 1
    'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  '"',  '~',  0x00, '|',  'Z',  'X',  'C',  'V',  // 2
    'B',  'N',  'M',  '<',  '>',  '?',  0x00, '*',  0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 3
    0x00, 0x00, 0x00, 0x00, 0x00, '7',  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',  '2',  '3',  // 4
    '0',  '.',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 5
};

// ASCII control codes, originally defined in ANSI X3.4.
// static uint8_t ascii_control_codes_keymap[] = {
//     /*
//      0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F    */
//     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0
//     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 1
//     0x04, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, // 2
//     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 3
//     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 4
//     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 5
// };

static bool is_shift_pressed = 0;
static bool is_ctrl_pressed = 0;

static status_t
ps2_initialize_keyboard_driver()
{
    outb(PS2_COMMAND_PORT, PS2_COMMAND_ENABLE_FIRST_PORT);

    return ALL_OK;
}

static uint8_t
scancode_to_ascii_char(uint8_t scancode)
{
    if (scancode > (sizeof(us_qwerty_ascii_keymap) / sizeof(us_qwerty_ascii_keymap[0]))) {
        return 0;
    }

    if (is_ctrl_pressed) {
        // TODO: Send a signal to the foreground process.
        return 0;
    }

    char c = is_shift_pressed ? us_qwerty_ascii_keymap_shifted[scancode] : us_qwerty_ascii_keymap[scancode];

    return c;
}

static void*
ps2_keyboard_interrupt_handler(struct interrupt_frame* frame)
{
    uint8_t scancode = inb(PS2_DATA_PORT);

    /*
     * From https://wiki.osdev.org/%228042%22_PS/2_Controller:
     * > Unfortunately, there is one problem to worry about. If you send a command to the PS/2 controller that involves
     * > a response, the PS/2 controller may generate IRQ1, IRQ12, or no IRQ (depending on the firmware) when it puts
     * > the "response byte" into the buffer. In all three cases, you can't tell if the byte came from a PS/2 device or
     * > the PS/2 controller. In the no IRQ case, you additionally will need to poll for the byte. Fortunately, you
     * > should never need to send a command to the PS/2 controller itself after initialisation (and you can disable
     * > IRQs and both PS/2 devices where necessary during initialisation).
     *
     * So, we'll just read the first scancode, and not do anything to read the second byte. If there's any issue,
     * uncomment the following line:
     */
    // uint8_t _ignore = inb(PS2_DATA_PORT);

    if (scancode & PS2_KEYBOARD_KEY_RELEASED) {
        scancode &= ~PS2_KEYBOARD_KEY_RELEASED;
        if (scancode == PS2_KEYBOARD_LEFT_SHIFT || scancode == PS2_KEYBOARD_RIGHT_SHIFT) {
            is_shift_pressed = false;
        }
        if (scancode == PS2_KEYBOARD_LEFT_CTRL || scancode == PS2_KEYBOARD_RIGHT_CTRL) {
            is_ctrl_pressed = false;
        }
        return 0;
    }

    if (scancode == PS2_KEYBOARD_LEFT_SHIFT || scancode == PS2_KEYBOARD_RIGHT_SHIFT) {
        is_shift_pressed = true;
        return 0;
    }
    if (scancode == PS2_KEYBOARD_LEFT_CTRL || scancode == PS2_KEYBOARD_RIGHT_CTRL) {
        is_ctrl_pressed = true;
        return 0;
    }

    return (void*)((uint32_t)scancode_to_ascii_char(scancode));
}

struct keyboard*
ps2_default_keyboard_driver()
{
    static struct keyboard keyboard = {
        .name = "PS/2 Default Keyboard",
        .initialize = ps2_initialize_keyboard_driver,
        .interrupt_handler = ps2_keyboard_interrupt_handler,
    };

    return &keyboard;
}
