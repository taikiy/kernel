#include "kernel.h"
#include "terminal/terminal.h"
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "config.h"
#include "disk/disk.h"
#include "disk/stream.h"
#include "string/string.h"
#include "fs/path_parser.h"
#include "fs/file.h"

static struct paging_4gb_chunk *paging_chunk = 0;

void kernel_main()
{
    // Initialize the heap. Currently allocates 100MB.
    kernel_heap_initialize();

    // Initialize the file systems
    file_system_initialize();

    // Search and initialize the disks
    disk_search_and_initialize();

    // Initialize the Interrupt Descriptor Table
    idt_initialize();

    // Enable paging
    paging_chunk = paging_new_4gb(PAGING_IS_WRITABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    uint32_t *kernel_page_directory = paging_4gb_chunk_get_directory(paging_chunk);
    paging_switch(kernel_page_directory);
    enable_paging();

    // Enable the system interrupts
    enable_interrupts();

    // Test: Write to the video memory
    terminal_initialize();
    print("Hello, World!\nYou are in Protected Mode!\n");

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

    // Test: Read from the disk stream to `buf`
    struct disk *current_disk;
    char buf[0x10];
    current_disk = get_disk(0);
    struct disk_stream *stream = disk_stream_new(current_disk->id);
    disk_stream_seek(stream, 0x210);
    disk_stream_read(stream, 0x10, buf);
    disk_stream_close(stream);

    print("End of kernel_main\n");
}
