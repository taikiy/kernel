#ifndef STRING_H
#define STRING_H

#include <stdbool.h>

int strlen(const char *ptr);
int strnlen(const char *ptr, int max);
int strcmp(const char *str1, const char *str2);
int atoi(const char *str);
bool isdigit(char c);

#endif
