#ifndef PS2_DEFAULT_KEYBOARD_H
#define PS2_DEFAULT_KEYBOARD_H

#include "keyboard.h"

#define PS2_COMMAND_PORT              0x64
#define PS2_COMMAND_ENABLE_FIRST_PORT 0xAE
#define PS2_DATA_PORT                 0x60

#define PS2_KEYBOARD_KEY_RELEASED 0x80

struct keyboard* ps2_default_keyboard_driver();

#endif
