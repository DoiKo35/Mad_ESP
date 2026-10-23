<div align="left">

<img src="https://github.com/DoiKo35/Mad_ESP/blob/main/website/img/logo.PNG?raw=true" alt="Mad_ESP Logo" width="180">

# Mad_ESP

### DIY Portable Console & PDA. Powered by ESP32. Open Hardware.

**A custom portable mini-console and Personal Digital Assistant based on the ESP32 platform.**

[![GitHub Repository](https://img.shields.io/badge/GitHub-Repository-white?style=for-the-badge&logo=github&logoColor=black)](https://github.com/DoiKo35/Mad_ESP)
[![Platform](https://img.shields.io/badge/Platform-ESP32-black?style=for-the-badge&logo=espressif)](https://www.espressif.com/)
[![Firmware Version](https://img.shields.io/badge/Version-Alpha_1.0.0-orange?style=for-the-badge)](https://github.com/DoiKo35/Mad_ESP)
[![Language](https://img.shields.io/badge/C++-white?style=for-the-badge&logo=html5&logoColor=black)](https://github.com/DoiKo35/Mad_ESP)

*Simple hardware. Classic retro aesthetics. Endless possibilities.*

</div>

---

## 📟 About / О проекте

**Mad_ESP** — это миниатюрная переносная консоль и карманный ассистент (PDA), собранный своими руками на базе модуля **ESP32 DevKit V1**. Проект сочетает в себе ретро-эстетику монохромных дисплеев, кастомную прошивку с маскотом (злым доберманом), мини-игры и полезные утилиты.

---

## 🛠 Hardware Components / Аппаратная часть

* **MCU:** ESP32 DevKit V1 (на гребенках)
* **Display:** SSD1306 OLED (128x64 пикселей, I2C)
* **Controls:** 4 тактовые кнопки (`L1`, `L2`, `R1`, `R2`)
* **Power Management:** Модуль заряда TP4056 (с защитой)
* **Battery:** Поддержка аккумуляторов (Lenovo BL210, LG BL-44jn или компактный FL701230 3.7V 230mAh)
* **Audio:** Пьезодинамик для звуков интерфейса, музыки и игр

---

## ✨ Firmware Features / Возможности прошивки (Alpha 1.0.0)

* **Startup Sequence:** 
  * Фирменная загрузочная анимация / бутлого с маскотом **Mad_ESP** (128x64).
  * Аппаратная стартовая мелодия (140 BPM, pitch -10) через пьезодинамик.
* **Status Bar (Статус-бар):**
  * Отображение заряда батареи в процентах и анимированная иконка процесса зарядки.
  * Индикаторы состояния Wi-Fi и Bluetooth.
* **PlayStation-style Menu:** Горизонтальная карусель приложений в стиле культовых консолей.
* **Localization (Локализация):** Полная поддержка языков (English, Русский с реальной кириллицей, Українська).
* **Settings App (Настройки):**
  * Смена языка интерфейса.
  * Включение/выключение Wi-Fi и Bluetooth.
  * Управление звуком пьезодинамика (системные звуки, навигация, игры).
  * Выбор бутлого (классический пес Mad_ESP или текстовый логотип).
  * Сброс настроек до заводских (Factory Reset).
* **System Info (О системе):**
  * Информация о версии прошивки (`Mad_ESP Alpha 1.0.0`).
  * Модель железа (`ESP32 DevKit V1`).
  * Датчик температуры устройства/батареи.
  * Прогресс-бар использования оперативной памяти (например, `106/520 KB`).
* **Power App (Питание):** Управление питанием устройства (Выключение, Перезапуск, Спящий режим для экономии заряда с пробуждением по нажатию любой кнопки).
* **Games App (Игры):**
  * *Классический Кликер*.
  * *Платформер/Бродилка* (в стиле террарии).
  * Меню паузы в играх с возможностью настройки, сохранения, загрузки и выхода.
* **Files App (Файлы):**
  * Чтение текстовых файлов (`.txt` — шпаргалки, книги, заметки).
  * Навигация и смена размера текста "на лету".
  * Беспроводная загрузка файлов с телефона по Wi-Fi/Bluetooth.

---

## 🎮 Controls / Управление

Навигация по системе и играм осуществляется с помощью 4 физических кнопок:

* **L1 / L2:** Влево / Листать назад / Перемещение по меню
* **R1:** Действие / Выбрать / ОК
* **R2:** Выход / Назад
* **Комбинация (L1 + L2):** Прыжок (в платформере)

---

## 📄 License

Check the repository for license information before redistributing or modifying project components.

---

<div align="center">

### Mad_ESP

**Built for makers. Inspired by classic technology.**

[Repository](https://github.com/DoiKo35/Mad_ESP) · [Report an Issue](https://github.com/DoiKo35/Mad_ESP/issues)

<sub>© Mad_ESP Project. All rights reserved.</sub>

</div>
