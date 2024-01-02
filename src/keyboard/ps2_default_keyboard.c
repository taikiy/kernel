#include "ps2_default_keyboard.h"
#include "io/io.h"
#include "terminal/terminal.h"

static uint8_t keyboard_scancode_set_one[] = {
    /*
     0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F    */
    0x00, 0x1B, '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=',  0x08, '\t', // 0
    'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '[',  ']',  0x0d, 0x00, 'A',  'S',  // 1
    'D',  'F',  'G',  'H',  'J',  'K',  'L',  ';',  '\'', '`',  0x00, '\\', 'Z',  'X',  'C',  'V',  // 2
    'B',  'N',  'M',  ',',  '.',  '/',  0x00, '*',  0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 3
    0x00, 0x00, 0x00, 0x00, 0x00, '7',  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',  '2',  '3',  // 4
    '0',  '.',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 5
};

static status_t
ps2_initialize_keyboard_driver()
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
        // TODO: handle key release. For now, we ignore key releases.
        return 0;
    }

    // TODO: handle special keys like shift, caps lock, etc.

    return (void*)((uint32_t)scancode_to_ascii_char(scancode));
}

struct keyboard*
ps2_default_keyboard_driver()
{
    static struct keyboard keyboard = {
        .name              = "PS/2 Default Keyboard",
        .initialize        = ps2_initialize_keyboard_driver,
        .interrupt_handler = ps2_keyboard_interrupt_handler,
    };

    return &keyboard;
}
