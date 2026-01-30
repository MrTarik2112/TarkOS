# Task List

- [x] Investigate build system (Makefile) and fix cross-compiler path
- [x] Fix QEMU run command (add `run-win` and `run-curses`)
- [x] Implement "Rainbow Debug" in kernel to isolate crash location
- [x] Debug Mouse Driver Crash (Stuck on Blue)
    - [x] Analyze `kernel/drivers/mouse.c`
    - [x] Fix potential issues in mouse initialization
    - [x] Verify fix with user
- [x] Cleanup Debug Code
    - [x] Remove Rainbow Debug from `kernel.c`
    - [x] Remove assembly debug from `boot.asm`
    - [x] Restore correct flags in `boot.asm`
- [x] Final Verification
    - [x] Confirm boot to desktop
    - [x] Check mouse/keyboard functionality
- [x] Update README.md with comprehensive documentation

