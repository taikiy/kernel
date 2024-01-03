#include "elf.h"

Elf32_Shdr*
get_section_header(Elf32_Ehdr* header)
{
    // The section header table is located at the section header (sh) offset specified in the ELF header.
    return (Elf32_Shdr*)((uint32_t)header + header->e_shoff);
}

Elf32_Phdr*
get_program_header(Elf32_Ehdr* header)
{
    // The program header table is located at the program header (ph) offset specified in the ELF header.
    return (Elf32_Phdr*)((uint32_t)header + header->e_phoff);
}

char*
get_section_name_string_table(Elf32_Ehdr* header)
{
    // The section name string table is located at the offset specified in the `e_shstrndx`'th section header table.
    return (char*)((uint32_t)header + get_section_header(header)[header->e_shstrndx].sh_offset);
}

/// @brief Returns the virtual address to which the system first transfers control, thus starting the process.
/// @param header The ELF header.
/// @return The virtual address to which the system first transfers control.  If the file has no associated entry point,
/// this member holds zero.
void*
get_elf_entry_address(Elf32_Ehdr* header)
{
    return (void*)header->e_entry;
}
