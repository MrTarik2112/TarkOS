/**
 * TarkOS - Printf Functions
 * Formatted output for debugging
 */

#ifndef _LIB_PRINTF_H
#define _LIB_PRINTF_H

#include <kernel/types.h>

/**
 * Format string into buffer (like sprintf)
 * @param buf Output buffer
 * @param fmt Format string
 * @param ... Arguments
 * @return Number of characters written
 */
int sprintf(char* buf, const char* fmt, ...);

/**
 * Format string with va_list
 */
int vsprintf(char* buf, const char* fmt, __builtin_va_list args);

#endif /* _LIB_PRINTF_H */
