/**
 * TarkOS - I/O Port Access Functions
 * Low-level hardware port read/write operations
 */

#ifndef _KERNEL_PORTS_H
#define _KERNEL_PORTS_H

#include <kernel/types.h>

/**
 * Write a byte to an I/O port
 */
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

/**
 * Write a word (16-bit) to an I/O port
 */
static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

/**
 * Write a double-word (32-bit) to an I/O port
 */
static inline void outl(uint16_t port, uint32_t value) {
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

/**
 * Read a byte from an I/O port
 */
static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/**
 * Read a word (16-bit) from an I/O port
 */
static inline uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/**
 * Read a double-word (32-bit) from an I/O port
 */
static inline uint32_t inl(uint16_t port) {
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/**
 * Wait for I/O operation to complete
 * Writes to unused port 0x80 (POST diagnostic port)
 */
static inline void io_wait(void) {
    outb(0x80, 0);
}

#endif /* _KERNEL_PORTS_H */
