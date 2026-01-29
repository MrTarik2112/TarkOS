/**
 * TarkOS - String Library Functions
 * Basic string and memory manipulation functions
 */

#ifndef _LIB_STRING_H
#define _LIB_STRING_H

#include <kernel/types.h>

/**
 * Copy memory from source to destination
 */
void* memcpy(void* dest, const void* src, size_t n);

/**
 * Set memory to a value
 */
void* memset(void* dest, int c, size_t n);

/**
 * Set memory to a 16-bit value (useful for framebuffer)
 */
void* memsetw(void* dest, uint16_t val, size_t n);

/**
 * Set memory to a 32-bit value (useful for framebuffer)
 */
void* memsetl(void* dest, uint32_t val, size_t n);

/**
 * Compare memory regions
 */
int memcmp(const void* s1, const void* s2, size_t n);

/**
 * Move memory (handles overlapping regions)
 */
void* memmove(void* dest, const void* src, size_t n);

/**
 * Get string length
 */
size_t strlen(const char* str);

/**
 * Copy string
 */
char* strcpy(char* dest, const char* src);

/**
 * Copy string with length limit
 */
char* strncpy(char* dest, const char* src, size_t n);

/**
 * Compare strings
 */
int strcmp(const char* s1, const char* s2);

/**
 * Compare strings with length limit
 */
int strncmp(const char* s1, const char* s2, size_t n);

/**
 * Concatenate strings
 */
char* strcat(char* dest, const char* src);

/**
 * Find character in string
 */
char* strchr(const char* str, int c);

/**
 * Find last occurrence of character in string
 */
char* strrchr(const char* str, int c);

/**
 * Convert integer to string
 */
char* itoa(int value, char* str, int base);

/**
 * Convert unsigned integer to string
 */
char* utoa(uint32_t value, char* str, int base);

#endif /* _LIB_STRING_H */
