# TarkOS v1.0 Makefile

# Cross compiler
CC = $(HOME)/opt/cross/bin/i686-elf-gcc
LD = $(HOME)/opt/cross/bin/i686-elf-ld
AS = nasm

# Flags
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-stack-protector
LDFLAGS = -T linker.ld -nostdlib -m elf_i386
ASFLAGS = -f elf32

# Files
KERNEL = build/kernel.elf
ISO = TarkOS.iso

all: $(ISO)

build:
	mkdir -p build

build/boot.o: boot/boot.asm | build
	$(AS) $(ASFLAGS) $< -o $@

build/kernel.o: kernel/kernel.c | build
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): build/boot.o build/kernel.o
	$(LD) $(LDFLAGS) -o $@ $^

$(ISO): $(KERNEL)
	mkdir -p iso/boot/grub
	cp $(KERNEL) iso/boot/kernel.elf
	cp iso/boot/grub/grub.cfg iso/boot/grub/grub.cfg 2>/dev/null || true
	grub-mkrescue -o $@ iso

run: $(ISO)
	@echo "Attempting to run TarkOS v1.8..."
	qemu-system-i386 -cdrom $(ISO) -m 512M -smp 3 -vga std -display gtk || \
	qemu-system-i386 -cdrom $(ISO) -m 512M -smp 3 -vga std -display sdl || \
	qemu-system-i386 -cdrom $(ISO) -m 512M -smp 3 -vga std -display curses || \
	qemu-system-i386 -cdrom $(ISO) -m 512M -smp 3 -vga std

clean:
	rm -rf build $(ISO)

.PHONY: all clean run
