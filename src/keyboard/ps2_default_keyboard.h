#ifndef PS2_DEFAULT_KEYBOARD_H
#define PS2_DEFAULT_KEYBOARD_H

#include "keyboard.h"

#define PS2_COMMAND_PORT              0x64
#define PS2_COMMAND_ENABLE_FIRST_PORT 0xAE

struct keyboard* ps2_default_keyboard();

#endif
