#include "kernel.h"
#include "terminal/terminal.h"
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "config.h"
#include "disk/disk.h"
#include "string/string.h"
#include "fs/path_parser.h"

static struct paging_4gb_chunk *paging_chunk = 0;

void kernel_main()
{
    // Initialize the heap. Currently allocates 100MB.
    kernel_heap_initialize();

    // Search and initialize the disks
    disk_search_and_initialize();

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

    // Test: Read from the disk 0, lba 0 (the first sector), 1 block (512 bytes) to `buf`
    struct disk *current_disk;
    char buf[512];
    current_disk = get_disk(0);
    disk_read_block(current_disk, 0, 1, buf);

    // Enable the system interrupts
    enable_interrupts();

    // Test the path parser
    struct path_root *root_path = path_parse("0:/bin/sh.exe", 0);
    if (root_path)
    {
        print("Root path: 0:/");
        print(root_path->first->name);
        print("/");
        print(root_path->first->next->name);
        print("\n");
    }

    print("End of kernel_main\n");
}
