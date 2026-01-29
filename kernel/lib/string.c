/**
 * TarkOS - String Library Implementation
 * Basic string and memory manipulation functions
 */

#include <lib/string.h>

/**
 * Copy memory from source to destination
 */
void* memcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    
    /* Optimize for aligned copies */
    if (((uintptr_t)d & 3) == 0 && ((uintptr_t)s & 3) == 0) {
        while (n >= 4) {
            *(uint32_t*)d = *(const uint32_t*)s;
            d += 4;
            s += 4;
            n -= 4;
        }
    }
    
    while (n--) {
        *d++ = *s++;
    }
    
    return dest;
}

/**
 * Set memory to a value
 */
void* memset(void* dest, int c, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    uint8_t val = (uint8_t)c;
    
    /* Optimize for aligned sets */
    if (((uintptr_t)d & 3) == 0 && n >= 4) {
        uint32_t val32 = val | (val << 8) | (val << 16) | (val << 24);
        while (n >= 4) {
            *(uint32_t*)d = val32;
            d += 4;
            n -= 4;
        }
    }
    
    while (n--) {
        *d++ = val;
    }
    
    return dest;
}

/**
 * Set memory to a 16-bit value
 */
void* memsetw(void* dest, uint16_t val, size_t n) {
    uint16_t* d = (uint16_t*)dest;
    while (n--) {
        *d++ = val;
    }
    return dest;
}

/**
 * Set memory to a 32-bit value
 */
void* memsetl(void* dest, uint32_t val, size_t n) {
    uint32_t* d = (uint32_t*)dest;
    while (n--) {
        *d++ = val;
    }
    return dest;
}

/**
 * Compare memory regions
 */
int memcmp(const void* s1, const void* s2, size_t n) {
    const uint8_t* p1 = (const uint8_t*)s1;
    const uint8_t* p2 = (const uint8_t*)s2;
    
    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    
    return 0;
}

/**
 * Move memory (handles overlapping regions)
 */
void* memmove(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    
    if (d < s) {
        /* Copy forward */
        while (n--) {
            *d++ = *s++;
        }
    } else if (d > s) {
        /* Copy backward */
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    }
    
    return dest;
}

/**
 * Get string length
 */
size_t strlen(const char* str) {
    size_t len = 0;
    while (*str++) {
        len++;
    }
    return len;
}

/**
 * Copy string
 */
char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

/**
 * Copy string with length limit
 */
char* strncpy(char* dest, const char* src, size_t n) {
    char* d = dest;
    while (n && (*d++ = *src++)) {
        n--;
    }
    while (n--) {
        *d++ = '\0';
    }
    return dest;
}

/**
 * Compare strings
 */
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const uint8_t*)s1 - *(const uint8_t*)s2;
}

/**
 * Compare strings with length limit
 */
int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) {
        return 0;
    }
    return *(const uint8_t*)s1 - *(const uint8_t*)s2;
}

/**
 * Concatenate strings
 */
char* strcat(char* dest, const char* src) {
    char* d = dest;
    while (*d) {
        d++;
    }
    while ((*d++ = *src++));
    return dest;
}

/**
 * Find character in string
 */
char* strchr(const char* str, int c) {
    while (*str) {
        if (*str == (char)c) {
            return (char*)str;
        }
        str++;
    }
    return (c == '\0') ? (char*)str : NULL;
}

/**
 * Find last occurrence of character in string
 */
char* strrchr(const char* str, int c) {
    const char* last = NULL;
    while (*str) {
        if (*str == (char)c) {
            last = str;
        }
        str++;
    }
    return (c == '\0') ? (char*)str : (char*)last;
}

/**
 * Convert integer to string
 */
char* itoa(int value, char* str, int base) {
    char* p = str;
    char* p1, *p2;
    uint32_t uval;
    int negative = 0;
    
    /* Handle sign for decimal */
    if (base == 10 && value < 0) {
        negative = 1;
        uval = (uint32_t)(-value);
    } else {
        uval = (uint32_t)value;
    }
    
    /* Generate digits in reverse order */
    do {
        int digit = uval % base;
        *p++ = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        uval /= base;
    } while (uval);
    
    /* Add negative sign */
    if (negative) {
        *p++ = '-';
    }
    
    *p = '\0';
    
    /* Reverse the string */
    p1 = str;
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1++ = *p2;
        *p2-- = tmp;
    }
    
    return str;
}

/**
 * Convert unsigned integer to string
 */
char* utoa(uint32_t value, char* str, int base) {
    char* p = str;
    char* p1, *p2;
    
    /* Generate digits in reverse order */
    do {
        int digit = value % base;
        *p++ = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        value /= base;
    } while (value);
    
    *p = '\0';
    
    /* Reverse the string */
    p1 = str;
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1++ = *p2;
        *p2-- = tmp;
    }
    
    return str;
}
