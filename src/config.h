#ifndef CONFIG_H
#define CONFIG_H

#define TOTAL_GDT_SEGMENTS 6

#define TOTAL_INTERRUPTS 512

#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10

#define KERNEL_STACK_ADDRESS 0x600000
#define HEAP_SIZE_BYTES 104857600 // 100MB heap size
#define HEAP_BLOCK_SIZE_BYTES 4096
// https://wiki.osdev.org/Memory_Map_(x86)
#define HEAP_TABLE_ADDRESS 0x00007E00 // We need 25,600 bytes for our table to store 100MB heap
#define HEAP_ADDRESS 0x01000000

// Requested Protection Level (RPL) allows software to override the CPL to select a new protection level.
// The Current Protection Level (CPL) is the protection level of the currently executing program. The CPL is stored in bits 0 and 1 of SS and CS.
// http://www.brokenthorn.com/Resources/OSDev23.html
#define USER_PROGRAM_CODE_SELECTOR 0x18 | 0x03 // user mode code selector is 0x18. With RPL 3 this is 0x1b
#define USER_PROGRAM_DATA_SELECTOR 0x20 | 0x03 // user mode data selector is 0x20. With RPL 3 this is 0x23

// Stack size (or any other memory allocation) must align to 4096 bytes (page size)
#define USER_PROGRAM_STACK_SIZE HEAP_BLOCK_SIZE_BYTES * 4
#define USER_PROGRAM_VIRTUAL_ADDRESS_START 0x400000
#define USER_PROGRAM_STACK_VIRTUAL_ADDRESS_START 0x3FF000
#define USER_PROGRAM_STACK_VIRTUAL_ADDRESS_END USER_PROGRAM_STACK_VIRTUAL_ADDRESS_START - USER_PROGRAM_STACK_SIZE

#define DISK_SECTOR_SIZE_BYTES 512

#define MAX_PATH_LENGTH 108

#define MAX_PROCESSES 10
#define MAX_ALLOCATIONS_PER_PROCESS 1024

#define MAX_FILE_SYSTEM_COUNT 16
#define MAX_FILE_DESCRIPTOR_COUNT 512 // Max number of open files

#endif
