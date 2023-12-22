#ifndef TSS_H
#define TSS_H

#include <stdint.h>

// Offset into the TSS entry in the `gdt_entries`.
// Our TSS entry is defined at the end of `gdt_entries`, so:
// (TOTAL_GDT_SEGMENTS - 1) * sizeof(gdt) = 40 bytes (0x28)
#define TSS_SEGMENT_OFFSET 0x28

struct tss
{
    uint32_t link; // previous task link
    uint32_t esp0; // kernel stack pointer
    uint32_t ss0;  // kernel stack segment
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3; // page directory pointer
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp; // user stack pointer
    uint32_t ebp; // user stack base pointer
    uint32_t esi;
    uint32_t edi;
    uint32_t es;   // user data segment
    uint32_t cs;   // user code segment
    uint32_t ss;   // user stack segment
    uint32_t ds;   // user data segment
    uint32_t fs;   // user data segment
    uint32_t gs;   // user data segment
    uint32_t ldtr; // local descriptor table register
    uint32_t iopb; // I/O Port Bitmap
} __attribute__((packed));

extern void load_tss(int offset);
void tss_initialize(struct tss *tss);

#endif
