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
#include "status.h"
#include "string/string.h"
#include "system/sys.h"
#include "system/syscall.h"
#include "task/process.h"
#include "task/task.h"
#include "task/tss.h"
#include "terminal/terminal.h"

void test_path_parser();
void test_file_system();
void test_user_space();
void test_syscall();

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
    initialize_kernel_space_paging();

    print("...IDT\n");
    initialize_idt();
    print("...ISR\n");
    initialize_interrupt_handlers();

    print("...disks and file systems\n");
    initialize_file_systems();
    initialize_disks();

    print("...keyboard drivers\n");
    initialize_keyboard_drivers();

    print("\nKernel is ready!\n\n");

    // TESTS
    // test_path_parser();
    // test_file_system();
    // test_user_space();
    test_syscall();

    print("...enabling interrupts\n");
    enable_interrupts();

    print("End of kernel_main\n");
}

void
test_syscall()
{
    struct process* proc = kzalloc(sizeof(struct process));
    print("Executing 0:/syscall.bin\n");
    status_t result = create_process_and_switch("0:/syscall.bin", &proc);
    if (result != ALL_OK || !proc) {
        panic("Failed to create a process!");
    }
}

void
test_user_space()
{
    struct process* proc = kzalloc(sizeof(struct process));
    print("Executing 0:/blank.bin\n");
    status_t result = create_process_and_switch("0:/blank.bin", &proc);
    if (result != ALL_OK || !proc) {
        panic("Failed to create a process!");
    }
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
    // Open a file. The file path contains the disk name, and the disk name is associated with a file system.
    // Resolving the file system is done in `initialize_disks()`.
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
