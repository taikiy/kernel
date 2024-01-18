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
void launch_shell();

void
kernel_main()
{
    terminal_initialize();
    print("Welcome to your own kernel!\n\n");

    print("Loading the Global Descriptor Table...");
    initialize_gdt();
    printc("   OK\n", GREEN);

    print("\nYou are in Protected Mode.\n\n");

    print("Initializing the kernel memory space...");
    initialize_kernel_heap();
    initialize_kernel_space_paging();
    printc("   OK\n", GREEN);

    print("Loading the Interrupt Descriptor Table...");
    initialize_idt();
    printc("   OK\n", GREEN);

    print("Registering Interrupt Service Routines...");
    initialize_interrupt_handlers();
    printc("   OK\n", GREEN);

    print("Loading disks and file systems...");
    initialize_file_systems();
    initialize_disks();
    printc("   OK\n", GREEN);

    print("Loading keyboard drivers...");
    initialize_keyboard_drivers();
    printc("   OK\n", GREEN);

    print("Enabling system interrupts...");
    enable_interrupts();
    printc("   OK\n", GREEN);

    print("\nKernel initialization complete!\n\n");

    launch_shell();
}

void
launch_shell()
{
    struct process* proc = 0;
    status_t result = create_process_and_switch("0:/shell", &proc);
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
