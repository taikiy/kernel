#ifndef FORMAT_ELF_H
#define FORMAT_ELF_H

#include "status.h"
#include <stddef.h>
#include <stdint.h>

typedef uint32_t Elf32_Addr; // Unsigned program address
typedef uint16_t Elf32_Half; // Unsigned medium integer
typedef uint32_t Elf32_Off;  // Unsigned file offset
typedef int32_t Elf32_Sword; // Signed large integer
typedef uint32_t Elf32_Word; // Unsigned large integer

// Program Header
// p_flags - Segment Flag Bits
#define PF_X        0x01
#define PF_W        0x02
#define PF_R        0x04
#define PF_MASKPROC 0xF0000000

// p_type - Segment Types
#define PT_NULL    0x00
#define PT_LOAD    0x01
#define PT_DYNAMIC 0x02
#define PT_INTERP  0x03
#define PT_NOTE    0x04
#define PT_SHLIB   0x05
#define PT_PHDR    0x06
#define PT_LOPROC  0x70000000
#define PT_HIPROC  0x7FFFFFFF

// sh_type - Section Types
#define SHT_NULL     0x00
#define SHT_PROGBITS 0x01
#define SHT_SYMTAB   0x02
#define SHT_STRTAB   0x03
#define SHT_RELA     0x04
#define SHT_HASH     0x05
#define SHT_DYNAMIC  0x06
#define SHT_NOTE     0x07
#define SHT_NOBITS   0x08
#define SHT_REL      0x09
#define SHT_SHLIB    0x0A
#define SHT_DYNSYM   0x0B
#define SHT_LOPROC   0x70000000
#define SHT_HIPROC   0x7FFFFFFF
#define SHT_LOUSER   0x80000000
#define SHT_HIUSER   0xFFFFFFFF

// e_type - Object File Types
#define ET_NONE   0x00
#define ET_REL    0x01
#define ET_EXEC   0x02
#define ET_DYN    0x03
#define ET_CORE   0x04
#define ET_LOPROC 0xFF00
#define ET_HIPROC 0xFFFF

// e_ident - ELF Identification
#define EI_MAG0    0
#define EI_MAG1    1
#define EI_MAG2    2
#define EI_MAG3    3
#define EI_CLASS   4
#define EI_DATA    5
#define EI_VERSION 6
#define EI_PAD     7
#define EI_NIDENT  16 // Size of e_ident[]

// ei_class - ELF Class
#define ELFCLASSNONE 0x00
#define ELFCLASS32   0x01
#define ELFCLASS64   0x02

// ei_data - ELF Data Encoding
#define ELFDATANONE 0x00
#define ELFDATA2LSB 0x01
#define ELFDATA2MSB 0x02

// e_shstrndx - Section Header String Table Index
#define SHN_UNDEF 0x00 // If the file has no section name string table, `e_shstrndx` holds the value SHN_UNDEF.

typedef struct
{
    Elf32_Word p_type;
    Elf32_Off p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
} __attribute__((packed)) Elf32_Phdr;

// Section Header
typedef struct
{
    Elf32_Word sh_name;
    Elf32_Word sh_type;
    Elf32_Word sh_flags;
    Elf32_Addr sh_addr;
    Elf32_Off sh_offset;
    Elf32_Word sh_size;
    Elf32_Word sh_link;
    Elf32_Word sh_info;
    Elf32_Word sh_addralign;
    Elf32_Word sh_entsize;
} __attribute__((packed)) Elf32_Shdr;

// ELF Header
typedef struct
{
    unsigned char e_ident[EI_NIDENT];
    Elf32_Half e_type;
    Elf32_Half e_machine;
    Elf32_Word e_version;
    Elf32_Addr e_entry;
    Elf32_Off e_phoff;
    Elf32_Off e_shoff;
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
} __attribute__((packed)) Elf32_Ehdr;

// Dynamic Structure
typedef struct
{
    Elf32_Sword d_tag;
    union
    {
        Elf32_Word d_val;
        Elf32_Addr d_ptr;

    } d_un;
} __attribute__((packed)) Elf32_Dyn;
extern Elf32_Dyn _DYNAMIC[];

// Symbol Table Entry
typedef struct
{
    Elf32_Word st_name;
    Elf32_Addr st_value;
    Elf32_Word st_size;
    unsigned char st_info;
    unsigned char st_other;
    Elf32_Half st_shndx;
} __attribute__((packed)) Elf32_Sym;

status_t load_elf_executable_file(int fd, void** out_ptr, size_t* out_size);

#endif
