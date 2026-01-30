# TarkOS v1.9.6 - Eternal Edition

32-bit hobby operating system written in C and x86 assembly. Features a graphical shell, file system, keyboard/mouse support, and games.

## Quick Start

### Prerequisites
- WSL (Windows Subsystem for Linux) with build tools
- QEMU emulator
- Cross-compiler (`i686-elf-gcc`)

### Build & Run
```bash
# Complete setup (first time)
chmod +x scripts/build_cross.sh
./scripts/build_cross.sh
echo 'export PATH="$HOME/opt/cross/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc

# Build
make clean
make

# Run
make run
```

## Features

### Core
- **Bootloader**: Multiboot-compliant GRUB boot
- **Memory**: PMM (Physical Memory Manager), Paging support
- **Interrupts**: GDT, IDT, ISR, PIC (Programmable Interrupt Controller)
- **VGA Driver**: 80x25 text mode with 16 colors

### Drivers
- **Keyboard**: PS/2 keyboard input with Shift support
- **Mouse**: PS/2 mouse with cursor tracking
- **Timer**: System timer and RTC (Real-Time Clock)

### GUI
- **Shell**: Command-line interface with persistent history
- **Window Manager**: Windowed UI with borders and shadows
- **Font Renderer**: Bitmap font rendering
- **Graphics**: Rectangle drawing and primitive UI elements

### File System
- **RAM Disk**: 16 files, 1KB each (in-memory)
- **Commands**: `ls`, `cat`, `rm`, `edit`, `history`, `clear`, `reboot`, `top`

### Applications
- **Text Editor**: Full-screen editor (Ctrl-S save, Ctrl-Q quit)
- **Snake Game**: Classic arcade game (WASD move, Q quit)
- **Number Game**: Guessing game
- **System Monitor**: Live uptime and resource display

## Commands

```
# File System
ls              List all files
cat FILE        Show file content
edit FILE       Create/edit file
rm FILE         Delete file
clear           Clear screen

# System
history         Show command history
top             System monitor
reboot          Restart OS
help            Show available commands
```

## Architecture

```
kernel/
  kernel.c          Main kernel entry
  arch/i386/        x86-specific code (GDT, IDT, ISR, PIC)
  drivers/          Keyboard, mouse, timer drivers
  gui/              VGA, graphics, window manager, font renderer
  mm/               Memory management (PMM, paging)
  lib/              printf, string utilities

boot/               Multiboot bootloader (NASM assembly)
include/            Public headers
iso/                GRUB ISO configuration
```

## Development

### Building
- `make` - Build TarkOS.iso
- `make clean` - Clean build artifacts
- `make run` - Run in QEMU with GTK/SDL display

### Cross-Compiler
Uses `i686-elf-gcc` for bare-metal i386 compilation:
- 32-bit Intel 80386 architecture
- No OS dependencies (freestanding environment)
- Custom linker script for memory layout

### System Specifications
- **Memory**: 512MB QEMU (configurable)
- **Cores**: 3-core SMP support
- **Resolution**: 80x25 text mode
- **Color Depth**: 4-bit (16 colors)

## Notes

- Files persist only in RAM (lost on reboot)
- Shift key enables uppercase characters
- Tab auto-completes to 4 spaces
- Screen auto-scrolls when full
- Mouse cursor visible in GUI

## License

MIT License - See LICENSE file

---

**TarkOS v1.9.6** | 18.0s Hyper-Cinematic Boot | Eternal Edition
