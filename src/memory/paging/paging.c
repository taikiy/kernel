#include "paging.h"
#include "memory/heap/kheap.h"

void paging_load_directory(uint32_t* directory);

static uint32_t* current_directory = 0;

struct paging_4gb_chunk*
paging_new_4gb(uint8_t flags)
{
    // allocate a linear memory space for a page directory (1024 entries)
    uint32_t* directory = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES);
    int offset = 0;

    for (int i = 0; i < PAGING_TOTAL_ENTRIES; i++) {
        // allocate a linear memory for a page table (1024 entries)
        uint32_t* table = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES);

        for (int j = 0; j < PAGING_TOTAL_ENTRIES; j++) {
            // set the page table entry
            // this is the virtual address of a 4K size page which is not allocated yet
            table[j] = (offset + j * PAGING_PAGE_SIZE_BYTES) | flags;
        }

        offset += (PAGING_TOTAL_ENTRIES * PAGING_PAGE_SIZE_BYTES);
        // set the page directory entry
        // this is the virtual address of the beginning of the page table at index `i`
        directory[i] = (uint32_t)table | flags | PAGING_IS_WRITABLE;
    }

    // this points to the first page directory entry, which will be fed to CR3 registry
    struct paging_4gb_chunk* chunk_4gb = kzalloc(sizeof(struct paging_4gb_chunk));
    chunk_4gb->directory = directory;

    return chunk_4gb;
}

status_t
paging_free_4gb(struct paging_4gb_chunk* chunk)
{
    status_t result = ALL_OK;

    if (!chunk) {
        result = ERROR(EINVARG);
        goto out;
    }

    for (int i = 0; i < PAGING_TOTAL_ENTRIES; i++) {
        uint32_t* table = (uint32_t*)(chunk->directory[i] & 0xfffff000); // mask the flags to get the table address
        kfree(table);
    }

    kfree(chunk->directory);
    kfree(chunk);

out:
    return result;
}

void
paging_switch(struct paging_4gb_chunk* chunk)
{
    paging_load_directory(chunk->directory);
    current_directory = chunk->directory;
}

static bool
paging_is_aligned(void* address)
{
    return ((uint32_t)address % PAGING_PAGE_SIZE_BYTES) == 0;
}

static status_t
paging_get_indexes(void* virtual_address, uint32_t* directory_index_out, uint32_t* table_index_out)
{
    status_t result = ALL_OK;

    if (!paging_is_aligned(virtual_address)) {
        result = ERROR(EINVARG);
        goto out;
    }

    *directory_index_out = (uint32_t)virtual_address / (PAGING_TOTAL_ENTRIES * PAGING_PAGE_SIZE_BYTES);
    *table_index_out =
      (uint32_t)virtual_address % (PAGING_TOTAL_ENTRIES * PAGING_PAGE_SIZE_BYTES) / PAGING_PAGE_SIZE_BYTES;

out:
    return result;
}

static uint32_t*
paging_get_aligned_address(void* address)
{
    return (uint32_t)address % PAGING_PAGE_SIZE_BYTES == 0
             ? address
             : (uint32_t*)(((uint32_t)address / PAGING_PAGE_SIZE_BYTES) * PAGING_PAGE_SIZE_BYTES +
                           PAGING_PAGE_SIZE_BYTES);
}

/// @brief Sets the `table_entry` value to the paging table, pointed by `virtual_address`, in the given `directory`.
/// @param directory Page Directory to set the value to.
/// @param virtual_address Absolute virtual address to the paging table entry.
/// @param table_entry The 20-bit address of the physical memory that the given virtual address will point to and the
/// page table entry flags.
/// @return Status code ALL_OK if success, or <0 if fails.
static status_t
paging_set(struct paging_4gb_chunk* chunk, void* virtual_address, uint32_t table_entry)
{
    if (!paging_is_aligned(virtual_address)) {
        return ERROR(EINVARG);
    }

    uint32_t directory_index = 0;
    uint32_t table_index = 0;
    status_t result = paging_get_indexes(virtual_address, &directory_index, &table_index);
    if (result < 0) {
        return result;
    }

    uint32_t directory_entry = chunk->directory[directory_index];
    uint32_t* table = (uint32_t*)(directory_entry & 0xfffff000);
    table[table_index] = table_entry;

    return ALL_OK;
}

static status_t
paging_map_4kb(struct paging_4gb_chunk* chunk, uint32_t* physical_address, uint32_t* virtual_address, uint32_t flags)
{
    return paging_set(chunk, virtual_address, (uint32_t)physical_address | flags);
}

status_t
map_physical_address_to_pages(
  struct paging_4gb_chunk* chunk,
  void* physical_address,
  void* virtual_address,
  uint32_t size,
  uint32_t flags
)
{
    status_t result = ALL_OK;

    if (!chunk || !chunk->directory || !physical_address || !virtual_address) {
        return ERROR(EINVARG);
    }

    if (size <= 0) {
        return ERROR(EINVARG);
    }

    uint32_t* physical_start_address = (uint32_t*)physical_address;
    uint32_t* physical_end_address = paging_get_aligned_address((void*)((uint32_t)physical_start_address + size));

    if (physical_start_address >= physical_end_address) {
        return ERROR(EINVARG);
    }

    if (!paging_is_aligned(physical_start_address) || !paging_is_aligned(physical_end_address) || !paging_is_aligned(virtual_address)) {
        return ERROR(EINVARG);
    }

    uint32_t total_pages = ((uint32_t)physical_end_address - (uint32_t)physical_start_address) / PAGING_PAGE_SIZE_BYTES;

    for (uint32_t i = 0; i < total_pages; i++) {
        result = paging_map_4kb(chunk, physical_address, virtual_address, flags);
        if (result < 0) {
            goto out;
        }
        physical_address += PAGING_PAGE_SIZE_BYTES;
        virtual_address += PAGING_PAGE_SIZE_BYTES;
    }

out:
    return result;
}
