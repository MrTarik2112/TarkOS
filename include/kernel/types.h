/**
 * TarkOS - Basic Type Definitions
 * Standard integer types and common macros for kernel development
 */

#ifndef _KERNEL_TYPES_H
#define _KERNEL_TYPES_H

/* Unsigned integer types */
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

/* Signed integer types */
typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef signed long long    int64_t;

/* Size types */
typedef uint32_t            size_t;
typedef int32_t             ssize_t;
typedef int32_t             ptrdiff_t;

/* Pointer-sized integer */
typedef uint32_t            uintptr_t;
typedef int32_t             intptr_t;

/* Boolean type */
typedef uint8_t             bool;
#define true                1
#define false               0

/* NULL pointer */
#ifndef NULL
#define NULL                ((void*)0)
#endif

/* Useful macros */
#define UNUSED(x)           ((void)(x))
#define ARRAY_SIZE(arr)     (sizeof(arr) / sizeof((arr)[0]))
#define ALIGN_UP(x, align)  (((x) + ((align) - 1)) & ~((align) - 1))
#define ALIGN_DOWN(x, align) ((x) & ~((align) - 1))

/* Bit manipulation */
#define BIT(n)              (1U << (n))
#define SET_BIT(x, n)       ((x) |= BIT(n))
#define CLEAR_BIT(x, n)     ((x) &= ~BIT(n))
#define TEST_BIT(x, n)      ((x) & BIT(n))

/* Min/Max macros */
#define MIN(a, b)           (((a) < (b)) ? (a) : (b))
#define MAX(a, b)           (((a) > (b)) ? (a) : (b))

/* Packed attribute for structs */
#define PACKED              __attribute__((packed))

/* Inline assembly helpers */
#define CLI()               __asm__ volatile("cli")
#define STI()               __asm__ volatile("sti")
#define HLT()               __asm__ volatile("hlt")
#define NOP()               __asm__ volatile("nop")

#endif /* _KERNEL_TYPES_H */
