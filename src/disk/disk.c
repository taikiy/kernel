#include "disk.h"
#include "io/io.h"
#include "config.h"
#include "memory/memory.h"

struct disk current_disk;

/// @brief Read `total` number of sectors (blocks) from the `lba` and store the read data to `buf`.
/// @param lba LBA (Logical Block Address) to read from
/// @param total Total blocks to read
/// @param buf A buffer to store the data
/// @return Status code
status_t disk_read_sector(unsigned int lba, unsigned int total, void *buf)
{
    // This is the same deal as in `boot.asm`.
    // Refer to `ata_lba_read` in https://wiki.osdev.org/ATA_read/write_sectors for more.
    // You can also find ATA commands here: https://wiki.osdev.org/ATA_Command_Matrix
    //
    // For example, `0x20` as you see in the line `outb(0x1F7, 0x20)` is `READ SECTORS` command.

    outb(0x1F6, (lba >> 24) | 0xE0);          // Send the highest 8 bits of the LBA to the disk controller
    outb(0x1F2, total);                       // Send the total sectors to read
    outb(0x1F3, (unsigned char)(lba & 0xff)); // Port to send bits 0-7 of LBA
    outb(0x1F4, (unsigned char)(lba >> 8));   // Port to send bits 8-15 of LBA
    outb(0x1F5, (unsigned char)(lba >> 16));  // Port to send bits 16-23 of LBA
    outb(0x1F7, 0x20);                        // Send "Read with retry" command to the command port

    // Cast the buffer pointer to `short`. The disk controller returns 1 word (2 bytes) at a time.
    unsigned short *ptr = (unsigned short *)buf;

    for (int b = 0; b < total; b++)
    {
        // Wait for the buffer to be ready
        char c = inb(0x1F7);
        while (!(c & 0x08))
        {
            c = inb(0x1F7);
        }

        // Ready to read. Copy 1 word from the hard disk to memory
        for (int i = 0; i < 256; i++) // 256 = 512 bytes per sector / 2 bytes read at a time
        {
            *ptr = inw(0x1F0); // Read from the data port
            ptr++;
        }
    }

    return ALL_OK;
}

void disk_search_and_initialize()
{
    memset(&current_disk, 0, sizeof(current_disk));
    current_disk.type = DISK_TYPE_REAL;
    current_disk.sector_size = DISK_SECTOR_SIZE_BYTES;
}

struct disk *get_disk(unsigned int disk_number)
{
    if (disk_number != 0)
    {
        return 0;
    }
    return &current_disk;
}

status_t disk_read_block(struct disk *disk, unsigned int lba, unsigned int total, void *buf)
{
    if (disk != &current_disk)
    {
        return -EIO;
    }
    return disk_read_sector(lba, total, buf);
}
