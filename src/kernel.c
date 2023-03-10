#include "kernel.h"
#include <stdint.h>
#include <stddef.h>
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "config.h"

uint16_t *video_mem = 0;
uint16_t terminal_row = 0;
uint16_t terminal_col = 0;

uint16_t terminal_make_char(char c, char color)
{
    return (color << 8) | c;
}

void terminal_put_char(int x, int y, char c, char color)
{
    video_mem[y * VGA_WIDTH + x] = terminal_make_char(c, color);
}

void terminal_write_char(char c, char color)
{
    if (c == '\n')
    {
        terminal_col = 0;
        terminal_row += 1;
        return;
    }

    terminal_put_char(terminal_col, terminal_row, c, color);
    terminal_col += 1;
    if (terminal_col >= VGA_WIDTH)
    {
        terminal_col = 0;
        terminal_row += 1;
    }
}

void terminal_initialize()
{
    video_mem = (uint16_t *)0xB8000;
    terminal_row = 0;
    terminal_col = 0;

    for (int y = 0; y < VGA_HEIGHT; y++)
    {
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            terminal_put_char(x, y, ' ', 0);
        }
    }
}

size_t strlen(const char *str)
{
    size_t len = 0;
    while (str[len])
    {
        len++;
    }
    return len;
}

void print(const char *str)
{
    size_t len = strlen(str);
    for (int i = 0; i < len; i++)
    {
        terminal_write_char(str[i], 15);
    }
}

static struct paging_4gb_chunk *paging_chunk = 0;

void kernel_main()
{
    // Initialize the heap. Currently allocates 100MB.
    kernel_heap_initialize();

    // Initialize the Interrupt Descriptor Table
    idt_initialize();

    // Enable paging
    paging_chunk = paging_new_4gb(PAGING_IS_WRITABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    uint32_t *kernel_page_directory = paging_4gb_chunk_get_directory(paging_chunk);
    paging_switch(kernel_page_directory);
    enable_paging();

    // Test: Write to the video memory
    terminal_initialize();
    print("Hello, World!\nYou are in Protected Mode!\n");

    // Test: Paging
    char *ptr = kzalloc(4096); // `ptr` points to some physical address
    // Map the virtual address 0x1000 (directory 0, table 1) to `ptr`, and make it writable by anyone
    paging_set(kernel_page_directory, (void *)0x1000, (uint32_t)ptr | PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT | PAGING_IS_WRITABLE);
    char *ptr2 = (char *)0x1000; // `ptr2` points to virtual address 0x1000
    ptr2[0] = 'A';
    ptr2[1] = 'B';
    ptr2[2] = '\n';
    print(ptr2); // prints the data at virtual address 0x1000
    print(ptr);  // prints the data at physical address `ptr`

    // Enable the system interrupts
    enable_interrupts();

    print("End of kernel_main\n");
}
