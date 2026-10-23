<div align="left">

<img src="https://github.com/DoiKo35/Mad_ESP/blob/main/website/img/logo.PNG?raw=true" alt="Mad_ESP Logo" width="180">

# Mad_ESP

### DIY Portable Gaming Console & PDA | Powered by ESP32 | Open Hardware

**A custom, ultra-portable mini gaming console and Personal Digital Assistant (PDA) based on the ESP32 platform.**

[![GitHub Repository](https://img.shields.io/badge/GitHub-Repository-white?style=for-the-badge&logo=github&logoColor=black)](https://github.com/DoiKo35/Mad_ESP)
[![Platform](https://img.shields.io/badge/Platform-ESP32-black?style=for-the-badge&logo=espressif)](https://www.espressif.com/)
[![Firmware Version](https://img.shields.io/badge/Version-Alpha_1.0.0-orange?style=for-the-badge)](https://github.com/DoiKo35/Mad_ESP)
[![Language](https://img.shields.io/badge/Language-C++-white?style=for-the-badge&logo=cplusplus&logoColor=black)](https://github.com/DoiKo35/Mad_ESP)

*Simple hardware. Classic monochrome aesthetic. Endless DIY possibilities.*

</div>

---

## 📟 About the Project

**Mad_ESP** is a handheld open-source DIY device combining retro gaming with useful PDA utilities. Built around the **ESP32 DevKit V1**, it features a unique Doberman mascot, a custom-crafted operating system, built-in games, text viewer capabilities, and full multi-language support.

---

## 🛠 Hardware Specifications

| Component | Description |
| :--- | :--- |
| **Microcontroller** | ESP32 DevKit V1 (mounted on female headers) |
| **Display** | 0.96" SSD1306 OLED (128x64 resolution, I2C interface) |
| **Controls** | 4 Tactile Micro-Buttons (`L1`, `L2`, `R1`, `R2`) |
| **Power Management** | TP4056 Lithium Charging Module (with protection circuitry) |
| **Battery Compatibility**| Li-Po / Li-ion (e.g., Lenovo BL210, LG BL-44jn, or ultra-compact FL701230 3.7V 230mAh) |
| **Audio** | Passive Piezo Buzzer for system alerts, 8-bit melodies, and game sound effects |

---

## ✨ Firmware Features (Alpha 1.0.0)

* **Custom Boot Sequence:**
  * Animated startup screen displaying the signature **Mad_ESP** mascot logo (128x64 bitmap).
  * Startup audio chime generated via the onboard piezo speaker.
* **Persistent Status Bar:**
  * Real-time battery percentage indicator with an active charging animation.
  * Status icons for active **Wi-Fi** and **Bluetooth** connections.
* **PS-Style Main Menu:** Smooth horizontal carousel interface inspired by classic gaming consoles.
* **Full Multi-Language Support:** Real Cyrillic rendering without transliteration — switch between **English**, **Russian** (Русский), and **Ukrainian** (Українська).
* **Settings Application:**
  * On-the-fly UI language selection.
  * Toggle Wi-Fi and Bluetooth radios.
  * Mute/unmute global audio (UI navigation, startup chime, in-game sound effects).
  * Toggle boot logo styles (Signature Mascot vs. Minimal Text).
  * Factory Reset option saved directly to non-volatile storage (EEPROM).
* **System Information App:**
  * Firmware version and hardware identifier.
  * Internal chip temperature sensor reading.
  * Real-time RAM utilization bar (e.g., `106 / 520 KB`).
* **Power Management App:**
  * Soft Shutdown, Reboot, and Power-Saving Sleep Mode (wakes up instantly on any button press).
* **Games Application:**
  * *Classic Clicker Game*.
  * *Platformer / World Explorer* (Terraria-inspired 2D mechanics).
  * In-game Pause menu with Save/Load state support, gameplay tweaks, and progress tracking.
* **File Reader Application:**
  * Read custom `.txt` files (cheat sheets, notes, books).
  * Adjust text font sizes dynamically on screen.
  * Wireless web-based file uploader via Wi-Fi/Bluetooth.

---

## 🎮 Navigation & Controls

System navigation and gameplay are streamlined across the 4 physical buttons:

* **`L1` / `L2`:** Move Left / Scroll Backwards / Navigate Menu Items
* **`R1`:** Action / Select / Confirm (OK)
* **`R2`:** Back / Exit / Cancel
* **`L1` + `L2` (Simultaneous):** Jump action (in Platformer game)

---

## 📄 License

Please review the repository for licensing terms before redistributing or modifying any hardware schematics or software code. 

If no specific license is declared, all rights remain with the original repository owner.

---

<div align="center">

### Mad_ESP

**Built for makers. Inspired by classic technology.**

[GitHub Repository](https://github.com/DoiKo35/Mad_ESP) · [Report an Issue](https://github.com/DoiKo35/Mad_ESP/issues)

<sub>© Mad_ESP Project. All rights reserved.</sub>

</div>
