#ifndef KERNEL_H
#define KERNEL_H

void kernel_main();
void switch_to_kernel_page();
void panic(const char* message);

#endif
