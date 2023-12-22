#ifndef GDT_H
#define GDT_H

#include <stdint.h>
#include "kernel.h"

/*
 * GDT segment structure:
 * +-------------------------------------------------------------+---------+---------+---------+---------------+---------------+
 * | 63   56 | 55   52 | 51   48 | 47         40 | 39         32 | 31                       16 | 15                          0 |
 * +---------+---------+---------+---------------+---------------+---------+---------+---------+---------------+---------------+
 * | Base(hi)|  Flags  | Limit(h)|    Access     |  Base (mid)   |         Base (low)          |          Limit (low)          |
 * | 31   24 | 3     0 | 19   16 | 7           0 | 23         16 | 15                        0 | 15                          0 |
 * +---------+---------+---------+---------------+---------------+---------+---------+---------+---------------+---------------+
 */
struct gdt
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct structured_gdt
{
    uint32_t base;
    uint32_t limit;
    uint8_t type; // Access byte (https://wiki.osdev.org/Global_Descriptor_Table)
};

extern void load_gdt(struct gdt *gdt, uint16_t size);

void initialize_gdt();
void structured_to_raw_gdt(struct structured_gdt *structured, struct gdt *raw, uint16_t entries);

#endif
