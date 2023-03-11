# Protected Mode Development 5 - Memory Management - Paging

## What is Paging?

[osdev wiki](https://wiki.osdev.org/Paging)

- Remaps memory addresses to point to other memory addresses
- creates an illusion of having the maximum amount of RAM installed
- hides memory from other processes

Paging works in 4096-byte block sizes by default. The blocks are called **pages**. When paging is enabled, the MMU (Memory Management Unit) will use the allocated page tables to resolve virtual addresses into physical addresses.

![x86 Paging Structure](https://wiki.osdev.org/images/7/77/Paging_Structure.gif)

### Paging Example

We could have a _Page Directory_ which contains 1024 entries. Each Page Directory entry points to a _Page Table_, which also contains 1024 entries. Each Page Table entry points to the start of a 4096-byte region in the physical memory address. So, Page Directory index 0x00 points to Page Table entries from index 0x00, 0x1000 (4096), 0x2000 (8192), to 0x3FF000 (1023 \* 4096). A Page Table 0x00 could point to any physical memory address of a 4KB block, or a _page_. The second index of the Page Directory will be 0x400000 (1024 \* 4096).

This allows us to address a maximum amount of RAM in a 32-bit system which is 4GB = 1024 directories \* 1024 tables \* 4KB. Note that these numbers are for 32-bit systems.

### Structures

#### Page Directory Entry

```
 +-----------------------------------------------------------------------+
 | Page Table Address | Empty | 0 | PS | 0 | A | CD | WT | U/S | R/W | P |
 +--------------------+-------+---+----+---+---+----+----+-----+-----+---+
32                    11      9                                          0
```

- `Page Table Address`\
  The physical 4096-byte aligned address to the start of a page table.
- `PS` - Page Size Bit\
  Set to 0 for 4KB pages, or 1 for 4MB pages.
- `A` - Accessed Bit\
  CPU sets this to 1 if this table is accessed.
- `CD` - Cache Disable Bit\
  Set to 1 to disable table caching.
- `WT` - Write-Through Bit\
  If this bit is set to 1, then write-through caching is enabled. If not, write-back is enabled.
- `U/S` - User/Supervisor Bit\
  If this is set to 1, then this table can be accessed by all privilege ring levels. If this is not set, then only supervisor ring levels can access this table.
- `R/W` - Read-Write Bit\
  If this bit is set to 1, the table is readable and writable. If this is not set, then this table is read-only. Note the WP bit in the CR0 register can allow writing in all cases for the supervisor.
- `P` - Present Bit\
  If this bit is set to 1, then the table exists in real memory. If this table is not available, the kernel set it to 0. If someone accesses this table, a _Page Fault_ will occur and the kernel is expected to resolve it.

#### Page Table Entry

Basically the same as Page Directory Entry structure + `G` flag.

```
 +-----------------------------------------------------------------------+
 |    Page Address    | Empty | G | 0 | D | A | CD | W/T | U/S | R/W | P |
 +--------------------+-------+---+---+---+---+----+-----+-----+-----+---+
32                    11      9                                          0
```

- `G` - Global
  Set to 1 to prevent TLB from updating the address in its cache if the CR3 register is reset.

### Page Fault

The CPU will call `Int 0x14`, the "Page Fault" exception, when there's a problem with paging. The exception is invoked when:

- a page in memory that does not have a `P` bit set is accessed.
- a page that has `U` bit set is accessed by a non-supervisor.
- a write occurs but the page is read-only and by a non-supervisor.

### Separating memory spaces

If we assign each process its own page **directory**, the process would see that the entire memory addresses are available for itself, and not see any other processes memory space. This means that there exist multiple page directories at the same time, but we can switch page directories when moving between processes. All processes can access the memory using the same memory addresses, but they will point to different physical addresses.

If a process tries to access a table/page that is not present, a page fault exception will occur. The kernel can load the data from hardware, network or whatever and store them back in memory, or swap, let that table/page entry's virtual address point to the physical address. Even a system with 100MB RAM can act as if it had access to a full 4GB.

### Enabling Paging

Paging is a feature provided by the CPU. We call assembly instructions and set the page directory/table to enable paging.

### Implementation

- Create directories and tables, and enable paging [[commit]()]
