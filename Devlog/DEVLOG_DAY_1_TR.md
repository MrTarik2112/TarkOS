# TarkOS - Geliştirici Günlüğü (1. Gün)

**Tarih:** 29 Ocak 2026  
**Geliştirici:** Tarık  
**Sürüm:** 0.3  
**Durum:** ✅ Çalışıyor!

---

## 🎯 Özet

Bugün sıfırdan bir işletim sistemi yazdım! Önce karmaşık bir grafik arayüzü denedim ama çok fazla sorunla karşılaştım. Sonunda basit ama güçlü bir text-mode OS'a döndüm ve harika çalışıyor.

---

## 🛠️ Teknik Özellikler

### Çekirdek (Kernel)
- **Dil:** C ve x86 Assembly
- **Mod:** 32-bit Protected Mode
- **Boyut:** ~350 satır kod
- **Multiboot:** GRUB uyumlu

### Donanım Sürücüleri
| Sürücü | Durum | Açıklama |
|--------|-------|----------|
| VGA Text | ✅ | 80x25 renkli metin modu |
| Klavye | ✅ | PS/2, Shift desteği |
| PIT Timer | ✅ | Uptime sayacı |

### Görsel Arayüz
- Üst başlık çubuğu (cyan arka plan)
- Alt durum çubuğu
- 16 renk desteği
- Kaydırma (scroll) özelliği
- Linux tarzı prompt: `tarkos@kernel:~$`

---

## 🎮 Komutlar

```
help    - Yardım menüsü
clear   - Ekranı temizle
info    - Sistem bilgisi
about   - ASCII logo ve hakkında
echo    - Yazı yazdır (echo merhaba)
calc    - Hesap makinesi (calc 5+3)
color   - Renk değiştir (color 14 1)
game    - Sayı tahmin oyunu
matrix  - Matrix animasyonu
art     - ASCII kedi çizimi
mem     - Bellek bilgisi
reboot  - Sistemi yeniden başlat
```

---

## 📅 Zaman Çizelgesi

| Saat | Aktivite |
|------|----------|
| 14:00 | Projeye başlandı, karmaşık GUI denendi |
| 15:00 | Mouse driver sorunu tespit edildi |
| 15:30 | Grafik mod Triple Fault verdi |
| 16:00 | Text mode'a geçiş kararı |
| 16:05 | "Hello World" başarıyla çalıştı! |
| 16:10 | v0.2 - Klavye + Shell eklendi |
| 16:12 | v0.3 - Oyun, animasyon, hesap makinesi |

---

## 🔧 Teknik Detaylar

### VGA Text Mode
```c
#define VGA_MEMORY 0xB8000
#define VGA_ENTRY(c, fg, bg) ((c) | (fg << 8) | (bg << 12))
```
Her karakter 2 byte: 1 byte ASCII, 1 byte renk.

### Klavye Sürücüsü
- Port 0x60 (data), 0x64 (status)
- Scancode tablosu ile ASCII dönüşümü
- Shift tuşu takibi

### Timer (PIT)
- Channel 0, Mode 3
- 100 Hz tick hızı
- Uptime sayacı için kullanılıyor

---

## 🐛 Çözülen Sorunlar

1. **Triple Fault:** VESA grafik modu QEMU'da çöküyordu
   - Çözüm: Text mode'a geçiş

2. **Mouse Crash:** PS/2 mouse initialization hang
   - Çözüm: Mouse tamamen devre dışı bırakıldı

3. **Cross-Compiler:** WSL'de PATH sorunu
   - Çözüm: Makefile'da tam yol kullanıldı

---

## 🚀 Gelecek Planlar

- [ ] Dosya sistemi (basit RAM disk)
- [ ] Daha fazla oyun (yılan, tetris)
- [ ] Metin editörü
- [ ] Çoklu görev (multitasking)
- [ ] Grafik modu (düzeltilmiş)

---

## 📸 Ekran Görüntüsü

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

## 💭 Sonuç

Bugün çok şey öğrendim:
- Basit başlamak her zaman daha iyi
- Text mode debug için harika
- Her şeyi bir günde yapmaya çalışma
- Çalışan küçük bir şey > Çalışmayan büyük bir şey

**Yarın:** Dosya sistemi ve daha fazla oyun!

---

*TarkOS - Sıfırdan yazılmış hobi işletim sistemi* 🖥️
