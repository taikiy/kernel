# What is an ELF file?

https://refspecs.linuxfoundation.org/elf/elf.pdf
https://wiki.osdev.org/ELF

ELF is a format for storing programs or fragments of programs on disk, created as a result of compiling and linking. An ELF file is divided into sections. For an executable program, these are the _text_ section for the code, the _data_ section for global variables and the _rodata_ section which usually contains constant strings. The ELF file contains headers that describe how these sections should be stored in memory. Because of this, multiple ELF files can be loaded and dynamically linked together at runtime into a single program in memory.

The kernel's ELF file loader is responsible for finding the symbols that are required by the program, resolving any conflicts, and loading them into memory. Linking is performed in memory at runtime.

- Type, struct, macro definitions [commit](https://github.com/taikiy/kernel/commit/4900ac6f21ac4fa42248265a22fb1ec1d65fb753)

## Loading ELF files

The ELF header contains all of the relevant information required to load an ELF executable. The format of this header is described in the [ELF Specification](http://www.skyfree.org/linux/references/ELF_Format.pdf). The most relevant sections for this purpose are 1.1 to 1.4 and 2.1 to 2.7. Instructions on loading an executable are contained within section 2.7.

The following is a rough outline of the steps that an ELF executable loader must perform:

1. Verify that the file starts with the ELF magic number (4 bytes) as described in Figure 1-4 (and subsequent table) on page 11 in the ELF specification.
2. Read the ELF Header. The ELF header is always located at the very beginning of an ELF file. The ELF header contains information about how the rest of the file is laid out. An executable loader is only concerned with the program headers.
3. Read the ELF executable's program headers. These specify where in the file the program segments are located, and where they need to be loaded into memory.
4. Parse the program headers to determine the number of program segments that must be loaded. Each program header has an associated type, as described in Figure 2-2 of the ELF specification. Only headers with a type of PT_LOAD describe a loadable segment.
5. Load each of the loadable segments. This is performed as follows:
   1. Allocate virtual memory for each segment, at the address specified by the p_vaddr member in the program header. The size of the segment in memory is specified by the p_memsz member.
   2. Copy the segment data from the file offset specified by the p_offset member to the virtual memory address specified by the p_vaddr member. The size of the segment in the file is contained in the p_filesz member. This can be zero.
   3. The p_memsz member specifies the size the segment occupies in memory. This can be zero. If the p_filesz and p_memsz members differ, this indicates that the segment is padded with zeros. All bytes in memory between the ending offset of the file size, and the segment's virtual memory size are to be cleared with zeros.
6. Read the executable's entry point from the ELF header.
7. Jump to the executable's entry point in the newly loaded memory.

- Helper functions for parsing ELF files [commit](https://github.com/taikiy/kernel/commit/3f1146fa06f0d881ecc74a7a82d2a8805d439481)
- Loading ELF files [commit]()
