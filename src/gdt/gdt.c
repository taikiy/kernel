#include "gdt.h"
#include "config.h"
#include "memory/memory.h"
#include "system/sys.h"
#include "task/tss.h"

struct tss tss;
struct gdt gdt_entries[TOTAL_GDT_SEGMENTS];
struct structured_gdt structured_gdt_entries[TOTAL_GDT_SEGMENTS] = {
    { .base = 0x00000000, .limit = 0x00000000, .type = 0x00 },      // NULL
    { .base = 0x00000000, .limit = 0xFFFFFFFF, .type = 0x9A },      // Kernel Code Segment
    { .base = 0x00000000, .limit = 0xFFFFFFFF, .type = 0x92 },      // Kernel Data Segment
    { .base = 0x00000000, .limit = 0xFFFFFFFF, .type = 0xFA },      // User Code Segment
    { .base = 0x00000000, .limit = 0xFFFFFFFF, .type = 0xF2 },      // User Data Segment
    { .base = (uint32_t)&tss, .limit = sizeof(tss), .type = 0x89 }, // TSS
};

extern void load_gdt(struct gdt* gdt, uint16_t size);

void
initialize_gdt()
{
    // Initialize the Global Descriptor Table
    memset(gdt_entries, 0, sizeof(gdt_entries));
    structured_to_raw_gdt(structured_gdt_entries, gdt_entries, TOTAL_GDT_SEGMENTS);
    load_gdt(gdt_entries, sizeof(gdt_entries));

    tss_initialize(&tss);
}

void
encode_gdt_entry(uint8_t* target, struct structured_gdt source)
{
    if (source.limit > 65536 && (source.limit & 0xFFF) != 0xFFF) {
        panic("encode_gdt_entry: GDT entry limit is too large");
    }

    // Encode the limit
    if (source.limit > 65536) {
        source.limit >>= 12;
        target[6] = 0xC0;
    } else {
        target[6] = 0x40;
    }
    target[0] = source.limit & 0xFF;
    target[1] = (source.limit >> 8) & 0xFF;
    target[6] |= (source.limit >> 16) & 0xF;

    // Encode the base
    target[2] = source.base & 0xFF;
    target[3] = (source.base >> 8) & 0xFF;
    target[4] = (source.base >> 16) & 0xFF;
    target[7] = (source.base >> 24) & 0xFF;

    // Encode the access
    target[5] = source.type;
}

void
structured_to_raw_gdt(struct structured_gdt* structured, struct gdt* raw, uint16_t entries)
{
    for (uint16_t i = 0; i < entries; i++) {
        encode_gdt_entry((uint8_t*)&raw[i], structured[i]);
    }
}
