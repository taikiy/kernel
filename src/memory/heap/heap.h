#ifndef HEAP_H
#define HEAP_H

#include "../../config.h"
#include "../../status.h"
#include <stddef.h>
#include <stdint.h>

#define HEAP_BLOCK_TABLE_ENTRY_FREE  0x00
#define HEAP_BLOCK_TABLE_ENTRY_TAKEN 0x01
#define HEAP_BLOCK_HAS_NEXT          0b1000000
#define HEAP_BLOCK_IS_FIRST          0b0100000

typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;

struct heap_table
{
    HEAP_BLOCK_TABLE_ENTRY* entries;
    size_t total;
};

struct heap
{
    struct heap_table* table;
    void* start_addr;
};

status_t heap_create(struct heap* heap, void* start, void* end, struct heap_table* table);
void* heap_malloc(struct heap* heap, size_t size);
status_t heap_free(struct heap* heap, void* ptr);

#endif
