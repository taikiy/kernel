#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>

void* malloc(size_t size);
void free(void* ptr);

// TODO: Move the functions below to stdio.h
void print(const char* str);
int getchar();
int putchar(int c);

#endif
