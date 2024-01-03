#include "elf_loader.h"
#include "memory/memory.h"
#include "stdbool.h"

const char ELF_MAGIC_NUMBER[4] = { ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3 };
elf_file file;

static Elf32_Ehdr*
get_elf_header(elf_file* file)
{
    // The ELF header is located at the beginning of the file.
    return (Elf32_Ehdr*)file->address;
}

static bool
has_magic_number(elf_file* file)
{
    // e_ident[0..3] == [0x7f, 'E', 'L', 'F']
    return memcmp(file->address, ELF_MAGIC_NUMBER, sizeof(ELF_MAGIC_NUMBER)) == 0;
}

static bool
is_supported_elf_file(elf_file* file)
{
    Elf32_Ehdr* header = get_elf_header(file);

    return (
      has_magic_number(file) && // Magic number
      // Our kernel only supports ELF files with the following properties:
      header->e_ident[EI_CLASS] == ELFCLASS32 &&               // 32-bit memory space
      header->e_ident[EI_DATA] == ELFDATA2LSB &&               // Little Endian
      header->e_type == ET_EXEC &&                             // Executable file
      header->e_entry >= USER_PROGRAM_VIRTUAL_ADDRESS_START && // Entry point in user space
      header->e_phoff != 0                                     // Program header table present
    );
}

status_t
load_elf_executable_file(int fd, void** out_ptr, size_t* out_size)
{
    status_t result = ALL_OK;

    return result;
}
