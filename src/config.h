#ifndef CONFIG_H
#define CONFIG_H

#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10

#define TOTAL_INTERRUPTS 512

#define HEAP_SIZE_BYTES 104857600 // 100MB heap size
#define HEAP_BLOCK_SIZE_BYTES 4096
// https://wiki.osdev.org/Memory_Map_(x86)
#define HEAP_TABLE_ADDRESS 0x00007E00 // We need 25,600 bytes for our table to store 100MB heap
#define HEAP_ADDRESS 0x01000000

#define DISK_SECTOR_SIZE_BYTES 512

#define MAX_PATH_LENGTH 108

#define MAX_FILE_SYSTEM_COUNT 16
#define MAX_FILE_DESCRIPTOR_COUNT 512 // Max number of open files

#endif
