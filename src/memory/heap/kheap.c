#include "kheap.h"
#include "../../config.h"
#include "../../status.h"
#include "../../system/sys.h"
#include "../../terminal/terminal.h"
#include "../memory.h"
#include "heap.h"

struct heap kernel_heap;
struct heap_table kernel_heap_table;

void
initialize_kernel_heap()
{
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*)HEAP_ADDRESS;
    kernel_heap_table.total = HEAP_SIZE_BYTES / HEAP_BLOCK_SIZE_BYTES;

    void* end = (void*)HEAP_ADDRESS + HEAP_SIZE_BYTES;
    status_t result = heap_create(&kernel_heap, (void*)HEAP_ADDRESS, end, &kernel_heap_table);

    if (result != ALL_OK) {
        panic("Failed to create heap\n");
    }
}

void*
kmalloc(size_t size)
{
    return heap_malloc(&kernel_heap, size);
}

void*
kzalloc(size_t size)
{
    void* ptr = kmalloc(size);

    if (!ptr) {
        return 0;
    }

    memset(ptr, 0, size);
    return ptr;
}

void
kfree(void* ptr)
{
    heap_free(&kernel_heap, ptr);
}
