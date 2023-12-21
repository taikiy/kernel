#include "kernel.h"
#include "terminal/terminal.h"
#include "idt/idt.h"
#include "io/io.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "config.h"
#include "disk/disk.h"
#include "disk/stream.h"
#include "string/string.h"
#include "fs/path_parser.h"
#include "fs/file.h"
#include "gdt/gdt.h"

static struct paging_4gb_chunk *paging_chunk = 0;

void test_path_parser();
void test_file_system();

void panic(const char *message)
{
    print("PANIC: ");
    print(message);
    print("\n");
    while (1)
    {
    };
}

struct gdt gdt_entries[TOTAL_GDT_SEGMENTS];
struct structured_gdt structured_gdt_entries[TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00000000, .limit = 0x00000000, .type = 0x00}, // NULL
    {.base = 0x00000000, .limit = 0xFFFFFFFF, .type = 0x9A}, // Kernel Code Segment
    {.base = 0x00000000, .limit = 0xFFFFFFFF, .type = 0x92}, // Kernel Data Segment
    {.base = 0x00000000, .limit = 0xFFFFFFFF, .type = 0xFA}, // User Code Segment
    {.base = 0x00000000, .limit = 0xFFFFFFFF, .type = 0xF2}, // User Data Segment
};

void kernel_main()
{
    // Initialize the Global Descriptor Table
    memset(gdt_entries, 0, sizeof(gdt_entries));
    structured_to_raw_gdt(structured_gdt_entries, gdt_entries, TOTAL_GDT_SEGMENTS);
    load_gdt(gdt_entries, sizeof(gdt_entries));

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

    // Bind the disk to FAT16
    struct disk *current_disk;
    current_disk = get_disk(0);
    fs_resolve(current_disk);

    // Initialize the terminal
    terminal_initialize();

    print("Hello, World! You are in Protected Mode.\n");

    // TESTS
    test_path_parser();
    test_file_system();

    print("End of kernel_main\n");
}

void test_path_parser()
{
    struct path_root *root_path = path_parse("0:/bin/sh.exe", 0);
    if (root_path)
    {
        print("Root path: 0:/");
        print(root_path->first->name);
        print("/");
        print(root_path->first->next->name);
        print("\n");
    }
}

void test_file_system()
{
    int fd = fopen("0:/hello.txt", "r");
    if (!fd)
    {
        print("ERROR: File not found!\n");
        return;
    }

    print("File opened successfully!\n");

    struct file_stat *stat = kzalloc(sizeof(struct file_stat));
    fstat(fd, stat);
    print("File size: ");
    print_int(stat->size);
    print("\n");

    // The content of hello.txt is " World! Hello,"
    // We'll read it in 2 parts:
    char buf[8];

    // Move the fd's position index to the 'H' of "Hello,"
    fseek(fd, 8, FILE_SEEK_CUR);

    // Read 6 bytes from the file
    fread(buf, 6, 1, fd);
    buf[7] = '\0';
    print(buf);

    // Move the fd's position index back to the beginning of the file
    fseek(fd, 0, FILE_SEEK_SET);

    // Read 7 bytes from the file
    fread(buf, 7, 1, fd);
    buf[8] = '\0';
    print(buf);

    print("\n");

    fclose(fd);
    print("File closed successfully!\n");
}
