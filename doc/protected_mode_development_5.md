# Protected Mode Development 5 - Memory Management - Heap

## The heap and memory allocation

Addressable memory in a 32-bit kernel is limited to 4,294,967,296 bytes or 4.29GB. When the system starts, the RAM is uninitialized. BIOS will start initializing parts of RAM for video and other hardware. Unused parts of RAM which start from _0x01000000_ are available to us. However, since 0xC0000000 is reserved, the memory array starting from 0x01000000 can give us a maximum of 3.22GB for a machine with 4GB or higher RAM installed.

As a kernel, it is responsible for allocating heap memory and keeping track of which regions are being used, freed, and so on.

### Example - The simplest possible heap implementation

1. `current_address` points to a start address that is free to use
2. a user calls `malloc` stores the address pointed by `current_address` to `tmp` variable
3. increment `current_address` by the allocated size
4. return `tmp`

Pro - Very easy to implement\
Con - No way to free memory so the system will run out of usable RAM at some point

### Our heap implementation

- We will have a table that describes which memory is taken, free, etc. We'll call it the "entry table"
- We will have a pointer to a piece of free memory, which will be allocated when `malloc` is called. We'll call it the "data pool".
- The heap will be allocated by blocks. Each block will be aligned to 4096 bytes = 4k bytes. If a user requests 50 bytes, 4096 bytes of memory will be returned.
- The entries for a single `malloc` call must be consecutive in the entry table.

Pro - Fast allocation/freeing of memory. Easy to implement.\
Con - Using blocks will result in wasted bytes. Memory fragmentation is possible. (we have enough free blocks, but since they are fragmented, they cannot be allocated.)

#### The Entry Table

- The entry table contains an array of 1-byte values that represent an entry (1 block) in the data pool.
- An array size will be the allocated heap size / 4096. If we allocate 100MB, 25600 bytes (entries = blocks).

#### The Entry structure

An entry is 1 byte and the structure is:

```
+------------------------------------------------------+
|          Flags           |         Entry Type        |
|--------------------------+---------------------------|
| HAS_N | IS_FIRST | 0 | 0 | ET_3 | ET_2 | ET_1 | ET_0 |
+------------------------------------------------------+
8                          4                           0
```

##### Flags

`HAS_N` - Set 1 if the entry to the right of us is part of our allocation\
`IS_FIRST` - Set 1 if this is the first entry of our allocation

##### Entry Types

`HEAP_BLOCK_TABLE_ENTRY_TAKEN` - The entry is taken and the address cannot be used\
`HEAP_BLOCK_TABLE_ENTRY_FREE` - The entry is free and may be used

#### The data pool

Simply a raw flat array of bytes that our heap implementation can give to users who need memory.

### Implement and debug the heap code

Use `qemu-system-i386` just so that we are truly in a 32-bit system. x86_64 is a 64-bit emulator.

[[commit](https://github.com/taikiy/kernel/commit/f3a3d3791ffef2e401f149ac237e63b037b9cac8)]

---

[previous](./protected_mode_development_4.md) | [next](./protected_mode_development_6.md)
