#include "tss.h"
#include "config.h"
#include "memory/memory.h"

void
tss_initialize(struct tss* tss)
{
    memset(tss, 0, sizeof(struct tss));
    tss->esp0 = KERNEL_STACK_ADDRESS;
    tss->ss0 = KERNEL_DATA_SELECTOR;

    load_tss(TSS_SEGMENT_OFFSET);
}
