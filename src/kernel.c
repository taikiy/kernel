#include "kernel.h"
#include "config.h"
#include "disk/disk.h"
#include "disk/stream.h"
#include "fs/file.h"
#include "fs/path_parser.h"
#include "gdt/gdt.h"
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "memory/paging/paging.h"
#include "string/string.h"
#include "task/tss.h"
#include "terminal/terminal.h"

void test_path_parser();
void test_file_system();

void
panic(const char* message)
{
    print("PANIC: ");
    print(message);
    print("\n");
    while (1) {
    };
}

static struct paging_4gb_chunk* paging_chunk = 0;

void
switch_to_kernel_page_directory()
{
    paging_chunk = paging_new_4gb(PAGING_IS_WRITABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    uint32_t* kernel_page_directory = paging_4gb_chunk_get_directory(paging_chunk);
    paging_switch(kernel_page_directory);
    enable_paging();
}

void
kernel_main()
{
    terminal_initialize();
    print("Welcome to your own kernel!\n\n");

    print("...GDT\n");
    initialize_gdt();
    print("You are in Protected Mode.\n");

    print("...kernel heap and paging\n");
    initialize_kernel_heap();
    switch_to_kernel_page_directory();

    print("...IDT\n");
    initialize_idt();

    print("...disks and file systems\n");
    initialize_file_systems();
    initialize_disks();

    print("...enabling interrupts\n");
    enable_interrupts();

    print("\nKernel is ready!\n\n");

    // TESTS
    test_path_parser();
    test_file_system();

    print("End of kernel_main\n");
}

void
test_path_parser()
{
    struct path_root* root_path = path_parse("0:/bin/sh.exe", 0);
    if (root_path) {
        print("Root path: 0:/");
        print(root_path->first->name);
        print("/");
        print(root_path->first->next->name);
        print("\n");
    }
}

void
test_file_system()
{
    // Bind the disk to FAT16
    struct disk* current_disk;
    current_disk = get_disk(0);
    fs_resolve(current_disk);

    int fd = fopen("0:/hello.txt", "r");
    if (!fd) {
        print("ERROR: File not found!\n");
        return;
    }

    print("File opened successfully!\n");

    struct file_stat* stat = kzalloc(sizeof(struct file_stat));
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
    buf[6] = '\0';
    print(buf);

    // Move the fd's position index back to the beginning of the file
    fseek(fd, 0, FILE_SEEK_SET);

    // Read 7 bytes from the file
    fread(buf, 7, 1, fd);
    buf[7] = '\0';
    print(buf);

    print("\n");

    fclose(fd);
    print("File closed successfully!\n");
}
