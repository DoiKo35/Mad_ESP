::: {align="left"}
![Mad_ESP
logo](https://github.com/DoiKo35/Mad_ESP/blob/main/website/img/logo.PNG?raw=true){width="120"}
\# Mad_ESP \### A DIY pocket computer powered by ESP32 A compact,
retro-inspired handheld device and custom operating system --- built by
makers, for makers.
\[\![Platform\](https://img.shields.io/badge/Platform-ESP32%20DevKit%20V1-black?style=flat-square&logo=espressif)\](https://www.espressif.com/)
\[\![Firmware\](https://img.shields.io/badge/Firmware-Alpha%201.0.0-white?style=flat-square)\](https://github.com/DoiKo35/Mad_ESP)
\[\![Language\](https://img.shields.io/badge/Firmware-C%2B%2B-blue?style=flat-square&logo=cplusplus&logoColor=white)\](https://isocpp.org/)
\*\*Classic handheld spirit. DIY hardware. A tiny world in your
pocket.\*\*
:::

\-\-- \## 📟 What is Mad_ESP? \*\*Mad_ESP\*\* is a DIY pocket computer /
mini handheld console based on the \*\*ESP32 DevKit V1\*\*. It combines
a small monochrome display, physical buttons, sound, wireless
connectivity, simple games, and practical tools in one compact device.
The project has its own identity: a black-and-white retro interface and
an angry-dog mascot. The device firmware and operating system are both
called \*\*Mad_ESP\*\*. \> \*\*Project status:\*\* Alpha concept. The
features below describe the intended design; not every feature is
necessarily implemented yet. \## 🧩 Planned Hardware \| Component \|
Purpose \| \|\-\--\|\-\--\| \| ESP32 DevKit V1 \| Main controller and
firmware platform \| \| SSD1306 OLED, 128 × 64 \| Monochrome screen \|
\| 4 tactile buttons \| Navigation and game controls \| \| Piezo buzzer
\| Startup melody and interface/game sounds \| \| TP4056 charging module
\| Li-ion/LiPo battery charging \| \| Small prototyping board \|
Soldered DIY assembly \| \| Pin headers \| Removable ESP32 module
mounting \| The battery is still to be selected. Candidate batteries
mentioned for the build are Lenovo BL210, LG BL-44JN, or a small 3.7 V
230 mAh FL701230 cell. \*\*Battery choice, charging current, wiring, and
protection must be verified against the exact battery and TP4056 board
before assembly.\*\* Do not connect a battery based on its name alone.
\## 🕹️ Controls The four physical buttons are named \*\*L1, L2, R1, and
R2\*\*. \| Button \| System navigation \| Platform game \|
\|\-\--\|\-\--\|\-\--\| \| L1 \| Previous item / scroll up \| Move left
\| \| L2 \| Next item / scroll down \| Move right \| \| L1 + L2 \| ---
\| Jump \| \| R1 \| Confirm / open \| Action \| \| R2 \| Back / exit \|
Pause / game menu \| The controls are intended to stay consistent across
the operating system and apps. Exact GPIO assignments will be
configurable in the firmware. \## 🖥️ Boot Experience The planned startup
sequence: 1. Show the Mad_ESP dog logo on the 128 × 64 SSD1306 display.
2. Play the startup melody once through the piezo buzzer, if sound is
enabled. 3. Initialize the actual system components and load settings.
4. Enter the main launcher when initialization is complete. The startup
screen is intended to use a custom monochrome bitmap. Users will be able
to choose between the classic dog-and-name logo and a simple
\*\*Mad_ESP\*\* text logo. \### Startup melody The melody is based on
the note sequence supplied for the project, with a target tempo of
\*\*140 BPM\*\* and pitch offset \*\*−10\*\*. The buzzer sound can be
disabled in Settings. \## 🧭 Main Launcher The launcher is planned as a
horizontal, console-style app carousel. Each app will have its own
simple pixel-style icon, with L1/L2 used to move between apps and R1 to
launch the selected app. Planned apps: - \*\*Games\*\* --- built-in
games. - \*\*Files\*\* --- text files and file transfer. -
\*\*Settings\*\* --- system, wireless, display, sound, and language
options. - \*\*Device Info\*\* --- firmware version, memory use, and
device information. - \*\*Power\*\* --- sleep, restart, and shutdown
options where supported. \## 🎮 Games The Games app is planned to
include: - \*\*Clicker\*\* --- a simple button-based incremental game. -
\*\*Platformer\*\* --- a small side-scrolling game inspired by classic
handheld games. The platformer is designed around the four-button
layout. Its pause menu is intended to support continuing, starting a new
game, saving, loading, changing game settings, and returning to the
launcher. Save/load support depends on the storage method implemented in
the firmware. \## 📁 Files The Files app is intended to let users open
and read text files such as \`.txt\` notes or study notes. Planned
features: - Browse available text files and select one by name. - Read
text on the SSD1306 screen. - Scroll through text with L1/L2. - Adjust
text size with R1 where supported by the reader UI. - Return to the file
list with R2. - Transfer files from a phone over Wi-Fi or Bluetooth.
Wireless transfer requires a suitable transfer protocol and a phone-side
workflow; it is a planned feature, not a claim that every phone can
transfer files automatically. \## ⚙️ Settings The Settings app is
planned to include: - \*\*Language:\*\* English, Русский, Українська. -
\*\*Wi-Fi:\*\* enable or disable. - \*\*Bluetooth:\*\* enable or
disable. - \*\*Sound:\*\* enable or disable startup, navigation, and
game sounds. - \*\*Boot logo:\*\* classic dog logo or Mad_ESP text. -
\*\*Factory reset:\*\* restore default settings. - \*\*Hardware
configuration:\*\* configurable GPIO assignments for buttons, display,
and buzzer. Russian and Ukrainian interfaces are intended to use real
Cyrillic characters, not transliteration. The firmware must include a
font with Cyrillic glyphs for those languages. \## 🔋 Power The Power
app is planned to offer: - \*\*Sleep:\*\* turn off the display to reduce
power use; a button press wakes the screen. - \*\*Restart:\*\* reboot
the device. - \*\*Shutdown:\*\* perform a software shutdown where the
hardware design supports it. An ESP32 development board may not
completely disconnect its own power through software alone. True
low-power operation depends on the board, power circuitry, and wake-up
configuration. \## 📊 Device Info The Device Info screen is planned to
show: - Device: \*\*ESP32 DevKit V1\*\* - Firmware: \*\*Mad_ESP Alpha
1.0.0\*\* - Free and used heap memory, with a progress bar and a value
such as \`106 / 520 KB\` when those values match the actual runtime
measurements. - Device temperature, if a reliable and meaningful sensor
reading is available. - Battery percentage and charging status, if
suitable battery-voltage sensing and charging-status wiring are
implemented. - Wi-Fi and Bluetooth enabled/disabled indicators.
\*\*Battery percentage and charging status need hardware support.\*\*
The ESP32 cannot know a battery\'s exact charge percentage from software
alone without measurement and calibration. The internal ESP32
temperature sensor measures the chip, not the battery; battery
temperature requires a separate sensor. \## 🔋 Status Bar The planned
system status bar will show: - Battery icon and estimated percentage. -
Charging indicator when charging status is available. - Wi-Fi status. -
Bluetooth status. These indicators will reflect actual hardware readings
and radio state once the corresponding circuits and firmware support are
in place. \## 🛠️ Firmware & Development The firmware is intended to be
written in \*\*C++\*\* for the ESP32 platform. Hardware pin assignments
should be kept in one configuration section so the build can be adapted
without searching through the entire codebase. Configuration values will
cover: - SSD1306 display connection and I²C address. - L1, L2, R1, and
R2 GPIO pins. - Piezo buzzer pin. - Optional battery-voltage input and
charging-status input. - Optional temperature sensor. - Default language
and sound settings. Exact pin numbers and sensor wiring should be
documented once the final circuit is chosen. \### Website The repository
also contains a project website with: - A monochrome retro design. - The
Mad_ESP logo and angry-dog mascot. - About, firmware, and community
gallery sections. - A floating navigation header. - An animated
meteor-shower background and subtle interface animations. - Responsive
layout for desktop and mobile screens. The website uses classic HTML and
CSS, with JavaScript for interactive effects. The \*\*device
firmware\*\*, however, is planned for \*\*C++\*\*. \## 📂 Repository
\`\`\`text Mad_ESP/ ├── website/ │ ├── index.html │ ├── img/ │ │ ├──
logo.PNG │ │ ├── dog.png │ │ └── community/ │ │ └── kirilbk.jpg │ └──
fonts/ │ ├── system.otf │ └── chalkduster.ttf ├── README.md └──
\[firmware source files --- as the firmware is added\] \`\`\` This is an
illustrative overview of the website assets and intended project layout;
the repository may contain additional files. \## 🚧 Roadmap - \[ \]
Establish the ESP32 + SSD1306 hardware wiring. - \[ \] Implement the
boot logo and one-time startup melody. - \[ \] Build the launcher and
shared L1/L2/R1/R2 navigation. - \[ \] Add language selection with
Cyrillic font support. - \[ \] Implement system settings and persistent
configuration. - \[ \] Add battery measurement and charging-state
support. - \[ \] Implement Device Info and memory usage display. - \[ \]
Create the Clicker and platformer games. - \[ \] Add text file browsing
and reading. - \[ \] Explore phone-to-device file transfer over
Wi-Fi/Bluetooth. - \[ \] Implement sleep and restart behavior
appropriate to the hardware. - \[ \] Document the final circuit, pinout,
and firmware build steps. \## 🔧 Safety Notes - Check the pinout of
every module before wiring it. - Use a compatible protected battery and
verify the TP4056 board\'s charging configuration. - Never short the
battery or connect it directly to an ESP32 GPIO. - Confirm voltage
levels and power requirements for the display, buzzer, and
battery-sensing circuit. - Test the circuit with current-limited power
where possible before final assembly. \## 🤝 Contributing Ideas, bug
reports, hardware experiments, and improvements are welcome. 1. Fork the
repository. 2. Create a branch for your changes. 3. Test hardware
changes carefully and document the wiring. 4. Submit a pull request
describing what changed. \## 📄 License A license file should be added
to the repository to clearly define how the firmware, website, and other
project assets may be used and redistributed. \-\--

::: {align="left"}
\*\*Mad_ESP --- a tiny DIY pocket computer with a mean little
mascot.\*\* \[View the repository\](https://github.com/DoiKo35/Mad_ESP)
· \[Report a bug\](https://github.com/DoiKo35/Mad_ESP/issues)
:::
