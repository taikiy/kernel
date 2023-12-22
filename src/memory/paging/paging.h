#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <status.h>

#define PAGING_CACHE_DISABLED 0b00010000  // CD flag
#define PAGING_WRITE_THROUGH 0b00001000   // WT flag
#define PAGING_ACCESS_FROM_ALL 0b00000100 // U/S flag
#define PAGING_IS_WRITABLE 0b00000010     // R/W flag
#define PAGING_IS_PRESENT 0b00000001      // P flag

#define PAGING_TOTAL_ENTRIES 1024 // for both directory and table
#define PAGING_PAGE_SIZE_BYTES 4096

struct paging_4gb_chunk
{
    uint32_t *directory;
};

void initialize_paging();
struct paging_4gb_chunk *paging_new_4gb(uint8_t flags);
uint32_t *paging_4gb_chunk_get_directory(struct paging_4gb_chunk *chunk);
void paging_switch(uint32_t *directory);
void enable_paging();
status_t paging_set(uint32_t *directory, void *virtual_address, uint32_t table_entry);

#endif
