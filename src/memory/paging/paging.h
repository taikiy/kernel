#ifndef PAGING_H
#define PAGING_H

#include "task/task.h"
#include <status.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PAGING_CACHE_DISABLED  0b00010000 // CD flag
#define PAGING_WRITE_THROUGH   0b00001000 // WT flag
#define PAGING_ACCESS_FROM_ALL 0b00000100 // U/S flag
#define PAGING_IS_WRITABLE     0b00000010 // R/W flag
#define PAGING_IS_PRESENT      0b00000001 // P flag

#define PAGING_TOTAL_ENTRIES   1024 // for both directory and table
#define PAGING_PAGE_SIZE_BYTES 4096

struct paging_map
{
    uint32_t* directory;
};

extern void set_kernel_segment_registers();
extern void set_user_segment_registers();

struct paging_map* new_paging_map(uint8_t flags);
status_t free_paging_map(struct paging_map* map);
status_t copy_data_from_user_space(struct task* task, void* src, void* dest, size_t size);
status_t map_physical_address_to_pages(
  struct paging_map* map,
  void* physical_address,
  void* virtual_address,
  uint32_t size,
  uint32_t flags
);
void initialize_kernel_space_paging();
void switch_to_kernel_page();
void switch_to_user_page();

#endif
