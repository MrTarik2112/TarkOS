# TarkOS - Developer Log (Day 1)

**Date:** January 29, 2026  
**Developer:** Tarık  
**Version:** 0.3  
**Status:** ✅ Working!

---

## 🎯 Summary

Today I built an operating system from scratch! Initially attempted a complex graphical UI but faced many issues. Eventually pivoted to a simple but powerful text-mode OS that works great.

---

## 🛠️ Technical Specifications

### Kernel
- **Language:** C and x86 Assembly
- **Mode:** 32-bit Protected Mode
- **Size:** ~350 lines of code
- **Multiboot:** GRUB compatible

### Hardware Drivers
| Driver | Status | Description |
|--------|--------|-------------|
| VGA Text | ✅ | 80x25 color text mode |
| Keyboard | ✅ | PS/2, Shift support |
| PIT Timer | ✅ | Uptime counter |

### User Interface
- Top header bar (cyan background)
- Bottom status bar
- 16 color support
- Scrolling capability
- Linux-style prompt: `tarkos@kernel:~$`

---

## 🎮 Available Commands

```
help    - Show help menu
clear   - Clear screen
info    - System information
about   - ASCII logo and info
echo    - Print text (echo hello)
calc    - Calculator (calc 5+3)
color   - Change colors (color 14 1)
game    - Number guessing game
matrix  - Matrix animation
art     - ASCII cat drawing
mem     - Memory information
reboot  - Restart system
```

---

## 📅 Timeline

| Time | Activity |
|------|----------|
| 14:00 | Started project, attempted complex GUI |
| 15:00 | Mouse driver issue identified |
| 15:30 | Graphics mode caused Triple Fault |
| 16:00 | Decision to switch to text mode |
| 16:05 | "Hello World" successfully running! |
| 16:10 | v0.2 - Added keyboard + shell |
| 16:12 | v0.3 - Games, animations, calculator |

---

## 🔧 Technical Details

### VGA Text Mode
```c
#define VGA_MEMORY 0xB8000
#define VGA_ENTRY(c, fg, bg) ((c) | (fg << 8) | (bg << 12))
```
Each character is 2 bytes: 1 byte ASCII, 1 byte color attributes.

### Keyboard Driver
- Port 0x60 (data), 0x64 (status)
- Scancode to ASCII conversion table
- Shift key state tracking

### Timer (PIT)
- Channel 0, Mode 3 (Square Wave)
- 100 Hz tick rate
- Used for uptime counter

---

## 🐛 Problems Solved

1. **Triple Fault:** VESA graphics mode crashed in QEMU
   - Solution: Switched to text mode

2. **Mouse Crash:** PS/2 mouse initialization hung
   - Solution: Completely disabled mouse driver

3. **Cross-Compiler:** PATH issues in WSL
   - Solution: Used absolute paths in Makefile

---

## 🚀 Future Plans

- [ ] File system (simple RAM disk)
- [ ] More games (snake, tetris)
- [ ] Text editor
- [ ] Multitasking
- [ ] Graphics mode (fixed)

---

## 📸 Screenshot

```
┌──────────────────────────────────────────────────────────────────────────────┐
│ TarkOS v0.3                                              Uptime: 00:05:23    │
├──────────────────────────────────────────────────────────────────────────────┤
│ Welcome to TarkOS v0.3!                                                      │
│ Type 'help' for commands, 'about' for info.                                  │
│                                                                              │
│ tarkos@kernel:~$ help                                                        │
│ === Available Commands ===                                                   │
│   help    - Show this help                                                   │
│   clear   - Clear screen                                                     │
│   game    - Number guessing game                                             │
│   ...                                                                        │
│                                                                              │
│ tarkos@kernel:~$ _                                                           │
├──────────────────────────────────────────────────────────────────────────────┤
│ F1:Help | Type 'help' for commands                                           │
└──────────────────────────────────────────────────────────────────────────────┘
```

---

## 💭 Lessons Learned

- Start simple, always
- Text mode is excellent for debugging
- Don't try to do everything in one day
- A small working thing > A large broken thing

**Tomorrow:** File system and more games!

---

## 📊 Code Statistics

| Component | Lines | Description |
|-----------|-------|-------------|
| kernel.c | 350 | Main kernel with all features |
| boot.asm | 30 | Minimal bootloader |
| linker.ld | 25 | Linker script |
| Makefile | 35 | Build system |
| **Total** | **440** | Complete OS |

---

*TarkOS - A hobby operating system built from scratch* 🖥️
