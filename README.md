<div align="left">

<img src="https://github.com/DoiKo35/Mad_ESP/blob/main/website/img/logo.PNG?raw=true" alt="Mad_ESP Logo" width="180">

# Mad_ESP

### DIY Portable Console & PDA | Powered by ESP32 | Open Hardware

**A custom portable mini-console and Personal Digital Assistant based on the ESP32 platform.**

[![GitHub Repository](https://img.shields.io/badge/GitHub-Repository-white?style=for-the-badge&logo=github&logoColor=black)](https://github.com/DoiKo35/Mad_ESP)
[![Platform](https://img.shields.io/badge/Platform-ESP32-black?style=for-the-badge&logo=espressif)](https://www.espressif.com/)
[![Firmware Version](https://img.shields.io/badge/Version-Alpha_1.0.0-orange?style=for-the-badge)](https://github.com/DoiKo35/Mad_ESP)
[![Language](https://img.shields.io/badge/Language-C++-white?style=for-the-badge&logo=cplusplus&logoColor=black)](https://github.com/DoiKo35/Mad_ESP)

*Simple hardware. Classic retro aesthetics. Endless possibilities.*

</div>

---

## 📟 About the Project

**Mad_ESP** is a miniature handheld console and Personal Digital Assistant (PDA) built from scratch using the **ESP32 DevKit V1** module. The project combines monochrome display retro aesthetics, custom firmware featuring a mascot (a mad Doberman), mini-games, and useful utilities.

---

## 🛠 Hardware Components

- **MCU:** ESP32 DevKit V1
- **Display:** SSD1306 OLED (128x64 pixels, I2C)
- **Controls:** 4 Tactile Buttons (`L1`, `L2`, `R1`, `R2`)
- **Power Management:** TP4056 Charging Module
- **Battery:** Support for various batteries (Lenovo BL210, LG BL-44jn, or compact FL701230 3.7V 230mAh)
- **Audio:** Piezo buzzer for interface sounds, music, and games

---

## ✨ Firmware Features (Alpha 1.0.0)

- **Startup Sequence:** 
  - Signature boot animation / bootlogo featuring the **Mad_ESP** mascot.
  - Hardware startup melody played through the piezo buzzer.
- **Status Bar:**
  - Battery charge percentage display and charging status indicator.
  - Wi-Fi and Bluetooth status icons.
- **PS-style Menu:** Horizontal application carousel styled after classic consoles.
- **Localization:** Full multi-language support (English, Russian, Ukrainian).
- **Settings App:**
  - UI language switching.
  - Wi-Fi and Bluetooth toggles.
  - Piezo buzzer sound control (system sounds, navigation, games).
  - Bootlogo switching.
  - Factory Reset.
- **System Info:**
  - Firmware version information.
  - Hardware model info.
  - Device/Battery temperature sensor.
  - RAM usage state (e.g., `106/520 KB`).
- **Power App:** Device power management (Power Off, Restart, Sleep mode for power saving).
- **Games App:**
  - *Classic Clicker*.
  - *Platformer / Explorer* (Terraria-style).
- **Files App:**
  - Text file reading (`.txt` — cheat sheets, books, notes).
  - Navigation and font size changing on the fly.
  - Wireless file uploading from phone via Wi-Fi/Bluetooth.

---

## 🎮 Controls

System and game navigation is performed using 4 physical buttons:

- **L1 / L2:** Left / Scroll back / Menu navigation
- **R1:** Action / Select / OK
- **R2:** Exit / Back

---

## 📄 License

Check the repository for license information before redistributing or modifying project components.

---

<div align="center">

### Mad_ESP

**Built for makers. Inspired by classic technology.**

[Repository](https://github.com/DoiKo35/Mad_ESP) · [Report an Issue](https://github.com/DoiKo35/Mad_ESP/issues)

<sub>Mad_ESP Project.</sub>

</div>
