@echo off
echo Starting TarkOS in QEMU...
if exist "C:\Program Files\qemu\qemu-system-i386.exe" (
    "C:\Program Files\qemu\qemu-system-i386.exe" -cdrom TarkOS.iso -m 128M -display sdl
) else (
    echo QEMU not found in standard path. Trying PATH...
    qemu-system-i386 -cdrom TarkOS.iso -m 128M
)
pause
