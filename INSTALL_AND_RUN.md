# TarkOS - Kurulum ve Çalıştırma Rehberi (WSL & QEMU)

Bu rehber, Windows Subsystem for Linux (WSL) üzerinde geliştirme ortamını kurup TarkOS'u derlemeniz ve çalıştırmanız için gereken adımları içerir.

## 1. WSL Gerekli Paketlerin Kurulumu

WSL terminalinizi (Ubuntu/Debian) açın ve aşağıdaki komutları sırasıyla çalıştırın. Bu komutlar derleme için gerekli araçları ve QEMU emülatörünü yükler.

```bash
sudo apt update
sudo apt install -y build-essential bison flex libgmp3-dev libmpc-dev \
    libmpfr-dev texinfo nasm xorriso mtools grub-pc-bin grub-common qemu-system-i386
```

## 2. Cross-Compiler (Çapraz Derleyici) Hazırlığı

İşletim sistemi geliştirirken, kullandığınız Linux dağıtımının kendi derleyicisini (gcc) değil, saf bir derleyiciyi (cross-compiler) kullanmanız gerekir.

Bu işlem bilgisayarınızın hızına bağlı olarak **20-40 dakika** sürebilir.

1.  Script'e çalışma izni verin:
    ```bash
    chmod +x scripts/build_cross.sh
    ```

2.  Derleyiciyi kurun:
    ```bash
    ./scripts/build_cross.sh
    ```

3.  Kurulum bittikten sonra, derleyicinin yolunu (PATH) sisteminize eklemeniz gerekir. Aşağıdaki satırı `~/.bashrc` dosyanızın en altına ekleyin:
    ```bash
    export PATH="$HOME/opt/cross/bin:$PATH"
    ```

4.  Terminali kapatıp açın veya ayarları yükleyin:
    ```bash
    source ~/.bashrc
    ```

## 3. TarkOS'u Derleme

Artık işletim sistemini derleyip `.iso` dosyasını oluşturabilirsiniz:

```bash
make clean
make
```

Eğer her şey yolunda giderse `TarkOS.iso` dosyası oluşacaktır.

## 4. Çalıştırma (QEMU ile)

TarkOS'u çalıştırmak için iki yönteminiz var:

### Yöntem A: Doğrudan WSL İçinde (WSLg Gerekir)
Eğer Windows 11 kullanıyorsanız veya WSL'de grafik arayüz (GUI) desteğiniz varsa:

```bash
make run
```

### Yöntem B: Windows Üzerinden QEMU ile (Önerilen Alternatif)
Eğer WSL içinde grafik sorunu yaşıyorsanız veya siyah ekran görüyorsanız, QEMU'yu Windows tarafında kurup WSL üzerinden tetikleyebilirsiniz.

1.  Windows'a [QEMU for Windows](https://www.qemu.org/download/#windows) indirin ve kurun.
    *   Genellikle `C:\Program Files\qemu` dizinine kurulur.
2.  Şu komutu çalıştırın:
    ```bash
    make run-win
    ```

Bu komut WSL içinden Windows'taki QEMU'yu çağırarak `TarkOS.iso` dosyasını çalıştıracaktır.

---

### Sık Karşılaşılan Sorunlar

*   **"i686-elf-gcc: command not found" hatası:**
    *   2. adımdaki `export PATH` işlemini doğru yaptığınızdan ve `source ~/.bashrc` komutunu çalıştırdığınızdan emin olun.
*   **"grub-mkrescue: command not found" hatası:**
    *   `xorriso` ve `mtools` paketlerinin yüklü olduğunu kontrol edin (1. adım).
*   **Siyah Ekran / Görüntü Gelmiyor:**
    *   Grafik kartı uyumsuzluğu olabilir. `Makefile` içindeki `run` komutuna `-vga std` parametresini ekledim, bu genelde sorunu çözer.
    *   `make run-win` kullanmayı deneyin.
