#include "heap.h"
#include "status.h"
#include "memory/memory.h"

static int heap_validate_alignment(void *ptr)
{
    return ((unsigned int)ptr % HEAP_BLOCK_SIZE_BYTES) == 0;
}

static status_t heap_validate_table(void *start, void *end, struct heap_table *table)
{
    status_t result = ALL_OK;

    size_t table_size = (size_t)(end - start);
    size_t total_blocks = table_size / HEAP_BLOCK_SIZE_BYTES;

    if (table->total != total_blocks)
    {
        result = ERROR(EINVARG);
        goto out;
    }

out:
    return result;
}

status_t heap_create(struct heap *heap, void *start, void *end, struct heap_table *table)
{
    status_t result = ALL_OK;

    if (!heap_validate_alignment(start) || !heap_validate_alignment(end))
    {
        result = ERROR(EINVARG);
        goto out;
    }

    memset(heap, 0, sizeof(struct heap));
    heap->start_addr = start;
    heap->table = table;

    if (!heap_validate_table(start, end, table))
    {
        goto out;
    }

    size_t table_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * table->total;
    memset(table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table_size);

out:
    return result;
}

static uint32_t heap_align_value_to_upper(uint32_t val)
{
    if (val % HEAP_BLOCK_SIZE_BYTES == 0)
    {
        return val;
    }

    return (val - (val % HEAP_BLOCK_SIZE_BYTES)) + HEAP_BLOCK_SIZE_BYTES;
}

static int heap_get_entry_type(HEAP_BLOCK_TABLE_ENTRY entry)
{
    // last four bits
    return entry & 0x0f;
}

int heap_get_start_block(struct heap *heap, uint32_t blocks)
{
    struct heap_table *table = heap->table;
    int bc = 0;
    int bs = -1;

    for (size_t i = 0; i < table->total; i++)
    {
        if (heap_get_entry_type(table->entries[i]) != HEAP_BLOCK_TABLE_ENTRY_FREE)
        {
            bc = 0;
            bs = -1;
            continue;
        }

        // mark the first free block
        if (bs == -1)
        {
            bs = i;
        }

        bc++;

        if (bc == blocks)
        {
            break;
        }
    }

    if (bs == -1)
    {
        return -ENOMEM;
    }

    return bs;
}

void *heap_block_to_address(struct heap *heap, uint32_t start_block)
{
    return heap->start_addr + (start_block * HEAP_BLOCK_SIZE_BYTES);
}

uint32_t heap_address_to_block(struct heap *heap, void *ptr)
{
    return (unsigned int)(ptr - heap->start_addr) / HEAP_BLOCK_SIZE_BYTES;
}

void heap_mark_blocks_taken(struct heap *heap, uint32_t start_block, uint32_t blocks)
{
    uint32_t end = start_block + blocks - 1;

    // TODO: assert

    HEAP_BLOCK_TABLE_ENTRY entry = HEAP_BLOCK_IS_FIRST | HEAP_BLOCK_TABLE_ENTRY_TAKEN;
    if (blocks > 1)
    {
        entry |= HEAP_BLOCK_HAS_NEXT;
    }

    for (uint32_t i = start_block; i <= end; i++)
    {
        heap->table->entries[i] = entry;

        entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN;
        if (i != end - 1)
        {
            entry |= HEAP_BLOCK_HAS_NEXT;
        }
    }
}

void heap_mark_blocks_free(struct heap *heap, uint32_t start_block)
{
    struct heap_table *table = heap->table;

    for (uint32_t i = start_block;; i++)
    {
        HEAP_BLOCK_TABLE_ENTRY entry = table->entries[i];
        table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;
        if (!(entry & HEAP_BLOCK_HAS_NEXT))
        {
            break;
        }
    }
}

void *heap_malloc_blocks(struct heap *heap, uint32_t blocks)
{
    void *address = 0;

    uint32_t start_block = heap_get_start_block(heap, blocks);
    if (start_block < 0)
    {
        goto out;
    }

    address = heap_block_to_address(heap, start_block);
    heap_mark_blocks_taken(heap, start_block, blocks);

out:
    return address;
}

void *heap_malloc(struct heap *heap, size_t size)
{
    size_t aligned_size = heap_align_value_to_upper(size);
    uint32_t total_blocks = aligned_size / HEAP_BLOCK_SIZE_BYTES;

    return heap_malloc_blocks(heap, total_blocks);
}

status_t heap_free(struct heap *heap, void *ptr)
{
    status_t result = ALL_OK;

    uint32_t start_block = heap_address_to_block(heap, ptr);
    if (!(heap->table->entries[start_block] & HEAP_BLOCK_IS_FIRST))
    {
        result = ERROR(EINVARG);
        goto out;
    }

    heap_mark_blocks_free(heap, start_block);

out:
    return result;
}
