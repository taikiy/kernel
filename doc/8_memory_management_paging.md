# Memory Management - Paging

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

- Create directories and tables, and enable paging [[commit](https://github.com/taikiy/kernel/commit/209f36409c1922a36a0a953d2d5eeaf9ecb49fc8)]
- Implement virtual address mapping to physical memory address [[commit]](https://github.com/taikiy/kernel/commit/0773f6420deb57dea8c856ffdc56251baa73269e)

### Debugging

Here, I will show how a physical address is mapped to a virtual address using the debugger.

```shell
❯ qemu-system-i386 -hda ./bin/os.bin -s -S &
❯ i386-elf-gdb
(gdb) add-symbol-file ./build/kernelfull.o 0x100000
add symbol table from file "./build/kernelfull.o" at
        .text_addr = 0x100000
(y or n) y
Reading symbols from ./build/kernelfull.o...
(gdb) target remote localhost:1234
Remote debugging using localhost:1234
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
0x0000fff0 in ?? ()
(gdb) b kernel.c:97
Breakpoint 1 at 0x100475: file ./src/kernel.c, line 97.
(gdb) b kernel.c:103
Breakpoint 2 at 0x1004aa: file ./src/kernel.c, line 103.
(gdb) c
Continuing.

Breakpoint 1, kernel_main () at ./src/kernel.c:97
97          char *ptr = kzalloc(4096); // `ptr` points to some physical address
(gdb) n
99          paging_set(kernel_page_directory, (void *)0x1000, (uint32_t)ptr | PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT | PAGING_IS_WRITABLE);
(gdb) p ptr
$1 = 0x1f02000 ""
```

Observe that `ptr`'s address is `0x1f02000`. Now I continue expecting that I'd break at line 103 which is my breakpoint 2...

```shell
(gdb) c
Continuing.

Breakpoint 1, kernel_main () at ./src/kernel.c:97
97          char *ptr = kzalloc(4096); // `ptr` points to some physical address
(gdb) n
99          paging_set(kernel_page_directory, (void *)0x1000, (uint32_t)ptr | PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT | PAGING_IS_WRITABLE);
(gdb) p ptr
$2 = 0x2745000 ""
```

Unexpectedly, the code stops at line 97 again. Stepping to the next line and the address of `ptr` is now pointing to `0x2745000`. I stepped into assembly instructions, and I think the system does some kind of a reset when I execute `or eax, 0x80000000` in `enable_paging`, and runs the kernel code from the beginning. I guess it makes sense because we don't want heap memory allocated before the paging enabled to be still pointing to physical addresses that could corrupt data if we ever write to them.

I continue again.

```shell
(gdb) c
Continuing.

Breakpoint 2, kernel_main () at ./src/kernel.c:103
103         char *ptr2 = (char *)0x1000; // `ptr2` points to address 0x1000
(gdb) n
104         ptr2[0] = 'A';
(gdb)
105         ptr2[1] = 'B';
(gdb)
106         ptr2[2] = '\n';
(gdb) p ptr2
$3 = 0x1000 <idt_descriptors+3808> "AB"
(gdb) p ptr
$4 = 0x2745000 "AB"
```

Now it breaks at line 103 as expected. `ptr2`'s address is `0x1000` and `ptr` still points at `0x2745000`. Because the paging is enabled in the system, writing to `ptr2` also writes to `ptr`.

When I print `ptr2`, the debugger shows `$3 = 0x1000 <idt_descriptors+3808>`, which seems like physical address `0x1000`, but **I believe** this is because the debugger first "peeks" at `0x1000` to get the metadata like the segment and offset, and then "resolves" the address to get the actual data.

Let's look into the page directory to find the virtual address `0x1000`. Virtual address `0x1000` should be located at page directory index 0, page table index 1.

```shell
(gdb) p kernel_page_directory
$7 = (uint32_t *) 0x1f05000
(gdb) x/4x 0x1f05000
0x1f05000:      0x01f06027      0x01f09007      0x01f0d007      0x01f11007
```

Page directory index 0 is the first one, `0x01f06027`. Remember that this is the page directory entry, and is a combination of the page table address (MSB 20 bits) and flags. Let's extract the address part.

```shell
(gdb) p/x kernel_page_directory[0] & 0xfffff000
$10 = 0x1f06000
```

`0x1f06000` looks like the page table address we want. Now we look into the table.

```shell
(gdb) x/4x 0x1f06000
0x1f06000:      0x00000007      0x02745067      0x00002007      0x00003007
```

The second one is our target, virtual address `0x1000`. Again, this is the table entry, so we extract the address and print its contents.

```shell
(gdb) p/x 0x02745067 & 0xfffff00
$11 = 0x2745000
(gdb) x/8c 0x2745000
0x2745000:      65 'A'  66 'B'  0 '\000'        0 '\000'        0 '\000'        0 '\000'        0 '\000'        0 '\000'
(gdb)
```

We can see that the page table entry for virtual address `0x1000` points to physical address `0x2745000`, and its content is "AB"!

---

[previous](./7_memory_management_heap.md) | [next](./9_file_system_disk_driver.md)
