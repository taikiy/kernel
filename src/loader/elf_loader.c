#include "elf_loader.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"

const char ELF_MAGIC_NUMBER[4] = { ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3 };
elf_file file;

static Elf32_Ehdr*
get_elf_header(void* file_ptr)
{
    // The ELF header is located at the beginning of the file.
    return (Elf32_Ehdr*)file_ptr;
}

static bool
is_supported_elf_file(Elf32_Ehdr* header)
{
    // Our kernel only supports ELF files with the following properties:
    return (
      header->e_ident[EI_CLASS] == ELFCLASS32 &&               // 32-bit memory space
      header->e_ident[EI_DATA] == ELFDATA2LSB &&               // Little Endian
      header->e_type == ET_EXEC &&                             // Executable file
      header->e_entry >= USER_PROGRAM_VIRTUAL_ADDRESS_START && // Entry point in user space
      header->e_phoff != 0                                     // Program header table present
    );
}

bool
is_elf_file(void* file_ptr)
{
    // e_ident[0..3] == [0x7f, 'E', 'L', 'F']
    return memcmp(file_ptr, ELF_MAGIC_NUMBER, sizeof(ELF_MAGIC_NUMBER)) == 0;
}

static status_t
process_program_loadable_segment(Elf32_Ehdr* e_header, Elf32_Phdr* p_header, struct program* out_program)
{
    status_t result = ALL_OK;

    out_program->text_physical_address_start = (void*)((uint32_t)e_header + p_header->p_offset);
    out_program->text_virtual_address_start = (void*)p_header->p_vaddr;
    out_program->text_size = p_header->p_filesz;

    // allocate the stack memory
    void* stack_ptr = kzalloc(USER_PROGRAM_STACK_SIZE);
    if (!stack_ptr) {
        result = ERROR(ENOMEM);
        goto out;
    }
    out_program->stack_physical_address_start = stack_ptr;
    out_program->stack_size = USER_PROGRAM_STACK_SIZE;                                        // default
    out_program->stack_virtual_address_start = (void*)USER_PROGRAM_STACK_VIRTUAL_ADDRESS_END; // stack grows downwards

out:
    if (result != ALL_OK) {
        if (stack_ptr) {
            kfree(stack_ptr);
        }
    }
    return result;
}

static status_t
process_elf_program_headers(Elf32_Ehdr* e_header, struct program* out_program)
{
    status_t result = ALL_OK;

    Elf32_Phdr* p_headers = get_program_header_table(e_header);
    for (int i = 0; i < e_header->e_phnum; i++) {
        Elf32_Phdr* p_header = &p_headers[i];

        switch (p_header->p_type) {
            case PT_LOAD:
                result = process_program_loadable_segment(e_header, p_header, out_program);
                break;
            default:
                // TODO: support other program header types
                return ERROR(EFILENOTSUPPORTED);
        }
    }

    return result;
}

status_t
load_elf_executable_file(void* file_ptr, size_t file_size, struct program* out_program)
{
    status_t result = ALL_OK;

    Elf32_Ehdr* e_header = get_elf_header(file_ptr);
    if (!is_supported_elf_file(e_header)) {
        return ERROR(EFILENOTSUPPORTED);
    }

    // TODO: parse other header attributes to support dynamic linking, etc.
    result = process_elf_program_headers(e_header, out_program);

    return result;
}
