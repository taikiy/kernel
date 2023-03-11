#include "paging.h"
#include "memory/heap/kheap.h"

void paging_load_directory(uint32_t *directory);

static uint32_t *current_directory = 0;

struct paging_4gb_chunk *paging_new_4gb(uint8_t flags)
{
    // allocate a linear memory space of 1024 directory entries
    uint32_t *directory = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES);
    int offset = 0;

    for (int i = 0; i < PAGING_TOTAL_ENTRIES; i++)
    {
        // allocate a linear memory space of 1024 table entries
        uint32_t *table = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES);

        for (int j = 0; j < PAGING_TOTAL_ENTRIES; j++)
        {
            // set the page table's virtual address | flags
            table[j] = (offset + j * PAGING_PAGE_SIZE) | flags;
        }

        offset += (PAGING_TOTAL_ENTRIES * PAGING_PAGE_SIZE);
        // set the page directory's virtual address | flags
        directory[i] = (uint32_t)table | flags | PAGING_IS_WRITABLE;
    }

    // this points to the first page directory entry, which will be fed to CR3 registry
    struct paging_4gb_chunk *chunk_4gb = kzalloc(sizeof(struct paging_4gb_chunk));
    chunk_4gb->directory_entry = directory;

    return chunk_4gb;
}

void paging_switch(uint32_t *directory)
{
    paging_load_directory(directory);
    current_directory = directory;
}

uint32_t *paging_4gb_chunk_get_directory(struct paging_4gb_chunk *chunk)
{
    return chunk->directory_entry;
}
