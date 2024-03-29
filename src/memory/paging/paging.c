#include "paging.h"
#include "../../system/sys.h"
#include "../heap/kheap.h"
#include "../memory.h"

extern void load_directory(uint32_t* directory);
extern void enable_paging();

static struct paging_map* kernel_page = 0;
static uint32_t* current_directory = 0;

static void
switch_page(struct paging_map* map)
{
    if (!map->directory) {
        panic("Paging map directory is null");
    }

    load_directory(map->directory);
    current_directory = map->directory;
}

static bool
page_is_aligned(void* address)
{
    return ((uint32_t)address % PAGING_PAGE_SIZE_BYTES) == 0;
}

static status_t
get_page_indexes(void* virtual_address, uint32_t* directory_index_out, uint32_t* table_index_out)
{
    status_t result = ALL_OK;

    if (!page_is_aligned(virtual_address)) {
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
get_page_aligned_address(void* address)
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
set_table_entry(struct paging_map* map, void* virtual_address, uint32_t table_entry)
{
    if (!page_is_aligned(virtual_address)) {
        return ERROR(EINVARG);
    }

    uint32_t directory_index = 0;
    uint32_t table_index = 0;
    status_t result = get_page_indexes(virtual_address, &directory_index, &table_index);
    if (result != ALL_OK) {
        return result;
    }

    uint32_t directory_entry = map->directory[directory_index];
    uint32_t* table = (uint32_t*)(directory_entry & 0xfffff000);
    table[table_index] = table_entry;

    return ALL_OK;
}

static status_t
get_table_entry(struct paging_map* map, void* virtual_address, uint32_t* table_entry_out)
{
    if (!map || !map->directory || !virtual_address || !table_entry_out) {
        return ERROR(EINVARG);
    }

    uint32_t directory_index = 0;
    uint32_t table_index = 0;
    status_t result = get_page_indexes(virtual_address, &directory_index, &table_index);
    if (result != ALL_OK) {
        return ERROR(EPAGEFAULT);
    }

    uint32_t directory_entry = map->directory[directory_index];
    uint32_t* table = (uint32_t*)(directory_entry & 0xfffff000);
    uint32_t table_entry = table[table_index];

    if (!(table_entry & PAGING_IS_PRESENT)) {
        return ERROR(EPAGEFAULT);
    }

    *table_entry_out = table_entry;

    return ALL_OK;
}

struct paging_map*
new_paging_map(uint8_t flags)
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
        directory[i] = (uint32_t)table | flags;
    }

    // this points to the first page directory entry, which will be fed to CR3 registry
    struct paging_map* map = kzalloc(sizeof(struct paging_map));
    map->directory = directory;

    return map;
}

status_t
free_paging_map(struct paging_map* map)
{
    status_t result = ALL_OK;

    if (!map) {
        result = ERROR(EINVARG);
        goto out;
    }

    for (int i = 0; i < PAGING_TOTAL_ENTRIES; i++) {
        uint32_t* table = (uint32_t*)(map->directory[i] & 0xfffff000); // mask the flags to get the table address
        kfree(table);
    }

    kfree(map->directory);
    kfree(map);

out:
    return result;
}

status_t
map_paging_addresses(
  struct paging_map* map,
  void* virtual_address,
  void* physical_address,
  uint32_t size,
  uint32_t flags
)
{
    status_t result = ALL_OK;

    if (!map || !map->directory || !physical_address || !virtual_address) {
        return ERROR(EINVARG);
    }

    if (size <= 0) {
        return ERROR(EINVARG);
    }

    uint32_t* physical_start_address = (uint32_t*)physical_address;
    uint32_t* physical_end_address = get_page_aligned_address((void*)((uint32_t)physical_start_address + size));

    if (physical_start_address >= physical_end_address) {
        return ERROR(EINVARG);
    }

    if (!page_is_aligned(physical_start_address) || !page_is_aligned(physical_end_address) || !page_is_aligned(virtual_address)) {
        return ERROR(EINVARG);
    }

    uint32_t total_pages = ((uint32_t)physical_end_address - (uint32_t)physical_start_address) / PAGING_PAGE_SIZE_BYTES;

    for (uint32_t i = 0; i < total_pages; i++) {
        uint32_t table_entry = (uint32_t)physical_address | flags;
        result = set_table_entry(map, virtual_address, table_entry);
        if (result != ALL_OK) {
            goto out;
        }
        physical_address += PAGING_PAGE_SIZE_BYTES;
        virtual_address += PAGING_PAGE_SIZE_BYTES;
    }

out:
    return result;
}

void
initialize_kernel_space_paging()
{
    kernel_page = new_paging_map(PAGING_IS_PRESENT | PAGING_IS_WRITABLE);
    switch_page(kernel_page);
    enable_paging();
}

void
switch_to_kernel_page()
{
    set_kernel_segment_registers();
    switch_page(kernel_page);
}

void
switch_to_user_page(struct task* task)
{
    set_user_segment_registers();
    switch_page(task->user_page);
}

/// @brief Copies the data from the user space to the kernel space.
/// @param task The task that contains the user space paging map.
/// @param src The user space virtual address to copy from.
/// @param dest The kernel space physical address to copy to.
/// @param size The size of the data to copy.
/// @return
status_t
copy_data_from_user_space(struct task* task, void* src, void* dest, size_t size)
{
    status_t result = ALL_OK;
    const size_t max_page_size_to_copy = PAGING_PAGE_SIZE_BYTES;

    if (!task || !task->user_page || !dest || !src) {
        return ERROR(EINVARG);
    }

    // TODO: allow copying more than 4KB
    if (size <= 0 || size > max_page_size_to_copy) {
        return ERROR(EINVARG);
    }

    // temporary kernel space memory to copy the data from the user space
    char* buf = kzalloc(size);
    if (!buf) {
        return ERROR(ENOMEM);
    }

    struct paging_map* user_page = task->user_page;

    // get the original table entry at address `buf` and save it for later
    uint32_t original_table_entry = 0;
    result = get_table_entry(user_page, buf, &original_table_entry);
    if (result != ALL_OK) {
        goto out;
    }

    // Map the user space virtual address `buf` (which points to some unknown physical address) to point to the kernel
    // space physical address `buf`. The memory space is set to be read-only.
    result = map_paging_addresses(user_page, buf, buf, max_page_size_to_copy, PAGING_IS_PRESENT);
    if (result != ALL_OK) {
        goto out;
    }

    // switch to the user space for a brief moment to be able to see the `src` address
    switch_to_user_page(task);
    memcpy(buf, src, size);
    switch_to_kernel_page();

    // restore the original table entry
    result = set_table_entry(user_page, buf, original_table_entry);
    if (result != ALL_OK) {
        goto out;
    }

    // copy the data from the temporary space to the `dest`
    memcpy(dest, buf, size);

out:
    if (buf) {
        kfree(buf);
    }
    return result;
}
