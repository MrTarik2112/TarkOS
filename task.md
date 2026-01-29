# Task List

- [x] Investigate build system (Makefile) and fix cross-compiler path
- [x] Fix QEMU run command (add `run-win` and `run-curses`)
- [x] Implement "Rainbow Debug" in kernel to isolate crash location
- [ ] Debug Mouse Driver Crash (Stuck on Blue) <!-- id: 0 -->
    - [ ] Analyze `kernel/drivers/mouse.c`
    - [ ] Fix potential issues in mouse initialization
    - [ ] Verify fix with user
- [ ] Cleanup Debug Code
    - [ ] Remove Rainbow Debug from `kernel.c`
    - [ ] Remove assembly debug from `boot.asm`
    - [ ] Restore correct flags in `boot.asm`
- [ ] Final Verification
    - [ ] Confirm boot to desktop
    - [ ] Check mouse/keyboard functionality
