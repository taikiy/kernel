# Writing kernel in C

## Writing C program in Protected Mode

Now, we start writing our kernel in C. [[commit](https://github.com/taikiy/kernel/commit/2d58737d86668579690729b80f80fa2a9d01d422)]

Debugging steps are the same as what we did in the [previous note](./protected_mode_development_2.md) "5. Build, run, and debug".

## Writing "Hello, World!" in Protected Mode

### What is Text Mode?

- Write ASCII to video memory
  - starting at address 0xB8000 for colors, or
  - starting at address 0xB0000 for monochrome
- Supports 16 unique colors, e.g.,
  - color number 1, Black, RGB = 0 0 0, HEX = 00 00 00
  - color number 2, Blue, RGB = 0 0 170, HEX = 00 00 AA
  - ...
  - color number 15, White, RGB = 255 255 255, HEX = FF FF FF
- No need to set individual screen pixels for printing characters
  - The video card will take the ASCII value and automatically reference it in a font table to output pixels for the letter.

Each character will take 2 bytes.

- Byte 0 = ASCII character
- Byte 1 = Color

Example: Display "AB" in white

```x86asm
0xB8000 = 'A'
0xB8001 = 0x0F
0xB8002 = 'B'
0xB8003 = 0x0F
```

### Writing to video memory in C

```c
char *video_mem = (char *)0xB8000;
video_mem[0] = 'A';
video_mem[1] = '2'; // Green
```

but, we can optimize this by using `uint16_t`

```c
#include <stdint.h>

...

uint16_t *video_mem = (uint16_t *)0xB8000;
video_mem[0] = 0x0241; // 'A' (65 decimal or 0x41 hex) + Green (0x02) in the little-endian format
```

and with a bit more helper functions, we can call `print` and write "Hello, World!".

[[commit](https://github.com/taikiy/kernel/commit/fa0fbabaf9c9cd93bcaff966fb54164fa3da3df6)]

---

[Previous](./3_32-bit_kernel.md) | [Next](./5_interrupt_descriptor_table.md) | [Home](../README.md)
