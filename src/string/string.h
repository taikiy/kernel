#ifndef STRING_H
#define STRING_H

#include <stdbool.h>

int strlen(const char *ptr);
int strnlen(const char *ptr, int max);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, int n);
int istrncmp(const char *str1, const char *str2, int n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, int n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, int n);
int atoi(const char *str);
char *itoa(int n, char *str, int base);
bool isdigit(char c);

#endif
