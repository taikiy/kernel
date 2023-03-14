#ifndef DISK_H
#define DISK_H

#include "status.h"

status_t disk_read_sector(int lba, int total, void *buf);

#endif
