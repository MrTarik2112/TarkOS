/**
 * TarkOS - PIT Timer Driver
 * Programmable Interval Timer for system timing
 */

#include <kernel/types.h>
#include <kernel/ports.h>
#include <kernel/idt.h>
#include <kernel/isr.h>

/* PIT I/O ports */
#define PIT_CHANNEL0    0x40
#define PIT_CHANNEL1    0x41
#define PIT_CHANNEL2    0x42
#define PIT_COMMAND     0x43

/* PIT frequency */
#define PIT_FREQUENCY   1193182

/* Timer tick counter */
static volatile uint32_t timer_ticks = 0;
static uint32_t timer_frequency = 0;

/**
 * Timer interrupt handler (IRQ0)
 */
static void timer_handler(registers_t* regs) {
    (void)regs;
    timer_ticks++;
}

/**
 * Initialize the PIT timer
 * @param frequency Desired frequency in Hz (e.g., 100 for 100 ticks/second)
 */
void timer_init(uint32_t frequency) {
    timer_frequency = frequency;
    
    /* Calculate divisor */
    uint32_t divisor = PIT_FREQUENCY / frequency;
    
    /* Send command byte: channel 0, lobyte/hibyte, rate generator */
    outb(PIT_COMMAND, 0x36);
    
    /* Send divisor */
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));
    
    /* Register interrupt handler */
    register_interrupt_handler(IRQ0, timer_handler);
}

/**
 * Get current tick count
 */
uint32_t timer_get_ticks(void) {
    return timer_ticks;
}

/**
 * Get timer frequency
 */
uint32_t timer_get_frequency(void) {
    return timer_frequency;
}

/**
 * Get uptime in seconds
 */
uint32_t timer_get_uptime_seconds(void) {
    if (timer_frequency == 0) return 0;
    return timer_ticks / timer_frequency;
}

/**
 * Sleep for specified number of ticks
 */
void timer_sleep_ticks(uint32_t ticks) {
    uint32_t end = timer_ticks + ticks;
    while (timer_ticks < end) {
        HLT();  /* Wait for next interrupt */
    }
}

/**
 * Sleep for specified number of milliseconds
 */
void timer_sleep_ms(uint32_t ms) {
    if (timer_frequency == 0) return;
    uint32_t ticks = (ms * timer_frequency) / 1000;
    if (ticks == 0) ticks = 1;
    timer_sleep_ticks(ticks);
}
