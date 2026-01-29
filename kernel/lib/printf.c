/**
 * TarkOS - Printf Implementation
 * Simple formatted output for debugging
 */

#include <lib/printf.h>
#include <lib/string.h>

/**
 * Format string with va_list
 */
int vsprintf(char* buf, const char* fmt, __builtin_va_list args) {
    char* p = buf;
    
    while (*fmt) {
        if (*fmt != '%') {
            *p++ = *fmt++;
            continue;
        }
        
        fmt++;  /* Skip '%' */
        
        /* Handle flags (simplified) */
        bool pad_zero = false;
        int width = 0;
        
        if (*fmt == '0') {
            pad_zero = true;
            fmt++;
        }
        
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0');
            fmt++;
        }
        
        switch (*fmt) {
            case 'd':
            case 'i': {
                int val = __builtin_va_arg(args, int);
                char tmp[16];
                itoa(val, tmp, 10);
                int len = strlen(tmp);
                while (width > len) {
                    *p++ = pad_zero ? '0' : ' ';
                    width--;
                }
                strcpy(p, tmp);
                p += len;
                break;
            }
            
            case 'u': {
                uint32_t val = __builtin_va_arg(args, uint32_t);
                char tmp[16];
                utoa(val, tmp, 10);
                int len = strlen(tmp);
                while (width > len) {
                    *p++ = pad_zero ? '0' : ' ';
                    width--;
                }
                strcpy(p, tmp);
                p += len;
                break;
            }
            
            case 'x':
            case 'X': {
                uint32_t val = __builtin_va_arg(args, uint32_t);
                char tmp[16];
                utoa(val, tmp, 16);
                int len = strlen(tmp);
                while (width > len) {
                    *p++ = pad_zero ? '0' : ' ';
                    width--;
                }
                strcpy(p, tmp);
                p += len;
                break;
            }
            
            case 'p': {
                uint32_t val = __builtin_va_arg(args, uint32_t);
                *p++ = '0';
                *p++ = 'x';
                char tmp[16];
                utoa(val, tmp, 16);
                int len = strlen(tmp);
                while (8 > len) {
                    *p++ = '0';
                    len++;
                }
                strcpy(p, tmp);
                p += strlen(tmp);
                break;
            }
            
            case 's': {
                const char* str = __builtin_va_arg(args, const char*);
                if (str == NULL) str = "(null)";
                while (*str) {
                    *p++ = *str++;
                }
                break;
            }
            
            case 'c': {
                char c = (char)__builtin_va_arg(args, int);
                *p++ = c;
                break;
            }
            
            case '%':
                *p++ = '%';
                break;
            
            default:
                *p++ = '%';
                *p++ = *fmt;
                break;
        }
        
        fmt++;
    }
    
    *p = '\0';
    return p - buf;
}

/**
 * Format string into buffer
 */
int sprintf(char* buf, const char* fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    int ret = vsprintf(buf, fmt, args);
    __builtin_va_end(args);
    return ret;
}
