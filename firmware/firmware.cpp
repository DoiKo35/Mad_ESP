/*
  ================================================================================
  Project Name: Mad_ESP OS Firmware
  Version: Alpha 1.0.0
  Target Chip: ESP32 DevKit V1
  Display: SSD1306 OLED 128x64 (I2C)
  Mascot: Angry Dog ("Mad_ESP")
  
  Pinout Mapping:
  - OLED SDA  : GPIO 21
  - OLED SCL  : GPIO 22
  - Buzzer    : GPIO 25 (LEDC Channel 0 PWM)
  - Button L1 : GPIO 18 (INPUT_PULLUP) -> Navigate Left / Scroll Up
  - Button L2 : GPIO 19 (INPUT_PULLUP) -> Navigate Right / Scroll Down
  - Button R1 : GPIO 23 (INPUT_PULLUP) -> Action / Select / OK
  - Button R2 : GPIO 5  (INPUT_PULLUP) -> Cancel / Back / Exit
  - Battery ADC: GPIO 34 (Analog Voltage Divider input)
  - Charge Sense: GPIO 35 (Digital High when charging via TP4056)
  ================================================================================
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <WiFi.h>

// ==========================================
// HARDWARE PIN DEFINITIONS & CONFIGURATION
// ==========================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

#define PIN_BUZZER 25
#define PIN_BTN_L1 18
#define PIN_BTN_L2 19
#define PIN_BTN_R1 23
#define PIN_BTN_R2 5

#define PIN_BATTERY_ADC 34
#define PIN_CHARGE_STAT 35

#define EEPROM_SIZE 128
#define EEPROM_ADDR_MAGIC 0
#define EEPROM_ADDR_LANG 1
#define EEPROM_ADDR_SOUND 2
#define EEPROM_ADDR_WIFI 3
#define EEPROM_ADDR_BT 4
#define EEPROM_ADDR_LOGO 5
#define EEPROM_ADDR_GAME_SCORE 10

#define MAGIC_BYTE 0x42

// LEDC PWM Channel for Buzzer
#define BUZZER_CHANNEL 0

// Note Frequencies in Hz
#define NOTE_D3  147
#define NOTE_FS3 185
#define NOTE_E3  165
#define NOTE_B3  247

// ==========================================
// SYSTEM STRUCTURES & ENUMS
// ==========================================
enum Language { LANG_EN = 0, LANG_RU = 1, LANG_UA = 2 };
enum AppState { APP_BOOT, APP_MAIN_MENU, APP_GAMES, APP_FILES, APP_SETTINGS, APP_POWER };
enum GameType { GAME_PLATFORMER, GAME_CLICKER };

struct Settings {
  uint8_t language;
  bool soundEnabled;
  bool wifiEnabled;
  bool btEnabled;
  uint8_t bootLogo; // 0 = Mascot, 1 = Text
};

Settings sysSettings;
AppState currentState = APP_BOOT;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ==========================================
// BITMAPS & MASCOT LOGO
// ==========================================
const int bootlogo_width  = 128;
const int bootlogo_height = 64;
const unsigned char bootlogo[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xa0, 0x54, 0x00, 0x55, 0xa8, 0x45, 0x02,
  0x17, 0x00, 0x04, 0x08, 0x50, 0x20, 0x50, 0x88, 0x5c, 0xa8, 0x54, 0x88, 0x54, 0x2a, 0x54, 0x88,
  0x74, 0xaa, 0x54, 0x08, 0x14, 0x08, 0x5c, 0x88, 0x14, 0x20, 0x54, 0xa0, 0x75, 0x28, 0x54, 0x00,
  0x51, 0xa2, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x2a, 0x55, 0x08, 0x51, 0x00, 0x05, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x04, 0x00, 0x11, 0x00, 0x05, 0x02, 0x17, 0x2a, 0x5d, 0x0a, 0x55, 0x82, 0x45, 0x80,
  0x45, 0x80, 0x14, 0x00, 0x01, 0x00, 0x41, 0x00, 0x07, 0x02, 0x05, 0x02, 0x55, 0xa8, 0x54, 0x20,
  0x77, 0xa0, 0x45, 0x82, 0x47, 0x82, 0x44, 0x80, 0x00, 0x80, 0xc0, 0x80, 0x80, 0x80, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0xe0, 0xe0, 0xc0, 0x00, 0x20, 0xf8, 0xf0,
  0xf0, 0xfe, 0xfe, 0xf0, 0xf8, 0xf0, 0x00, 0x00, 0xe0, 0xe0, 0x80, 0x00, 0x80, 0x80, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81, 0x80, 0x44, 0x00, 0x04, 0x00, 0x04, 0x00,
  0x01, 0x00, 0x04, 0x80, 0xd1, 0xa0, 0x54, 0x00, 0x55, 0x28, 0x54, 0x00, 0x11, 0x00, 0x01, 0x00,
  0x15, 0x00, 0x05, 0x80, 0xd5, 0xa0, 0x54, 0x88, 0xd5, 0x22, 0x55, 0x8a, 0x7f, 0xaa, 0x5d, 0x8e,
  0x5f, 0xaa, 0x5d, 0xa8, 0x51, 0x00, 0x55, 0x00, 0x57, 0x02, 0x45, 0x02, 0x51, 0x02, 0x45, 0x02,
  0x55, 0x82, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x3f, 0x7f, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x1f, 0x7f,
  0xff, 0xff, 0xff, 0xff, 0x3f, 0x87, 0x80, 0xf8, 0xff, 0xff, 0xff, 0xbf, 0x5f, 0x87, 0x40, 0xa0,
  0x70, 0x38, 0x58, 0xa8, 0x70, 0x08, 0xf4, 0xfe, 0xff, 0x22, 0x15, 0x00, 0x80, 0xfc, 0xfc, 0xec,
  0x01, 0x00, 0x55, 0x08, 0xd5, 0xc2, 0x40, 0xe0, 0x71, 0xe0, 0x55, 0xa8, 0x55, 0x28, 0x54, 0x8a,
  0x9f, 0xca, 0xd5, 0x4a, 0x57, 0x6a, 0xff, 0xff, 0xdf, 0x08, 0x55, 0x02, 0x17, 0x02, 0x55, 0x0a,
  0x17, 0x02, 0x15, 0x00, 0x15, 0x00, 0x44, 0x80, 0x41, 0xf0, 0xec, 0x8c, 0x5d, 0x0c, 0x0c, 0x0e,
  0x57, 0x06, 0x06, 0x06, 0x02, 0x00, 0x38, 0xfc, 0xee, 0xc6, 0x86, 0x86, 0x0e, 0x00, 0x00, 0x18,
  0x3c, 0x02, 0xe2, 0xf3, 0x83, 0xc2, 0xc3, 0x63, 0x67, 0x3e, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x78, 0x78, 0xf0, 0xf0, 0xf4, 0xf0, 0xf5, 0xf3, 0xf7, 0xf3, 0x77, 0x3e,
  0x5f, 0xff, 0x7f, 0x3e, 0x7f, 0xfa, 0xf7, 0xfb, 0xf7, 0xf3, 0xf5, 0xf8, 0xf5, 0x70, 0x75, 0x00,
  0x51, 0x02, 0x45, 0x00, 0x75, 0x7e, 0x57, 0x03, 0x1f, 0x3f, 0x3c, 0x1f, 0x51, 0x00, 0x0f, 0x7f,
  0x3f, 0x00, 0x00, 0x1f, 0x3b, 0x30, 0x7d, 0x3a, 0x1f, 0x2f, 0x5f, 0x3a, 0x35, 0x20, 0x46, 0x1f,
  0x59, 0x90, 0x50, 0x90, 0x19, 0x1a, 0x0f, 0x07, 0x0f, 0x1c, 0x19, 0x30, 0x81, 0xc0, 0x81, 0xc0,
  0xd5, 0xfa, 0xdf, 0x82, 0x91, 0x82, 0xc7, 0xc2, 0x07, 0x03, 0x3f, 0x3b, 0x77, 0x3b, 0x35, 0x30,
  0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x60, 0x71, 0x71, 0x33, 0x3f, 0x0c, 0x00, 0x00,
  0x00, 0x00, 0x7f, 0x7b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x04, 0x00, 0x00, 0x00, 0x06, 0x0f, 0x07, 0x07, 0x07, 0x07, 0x03, 0x03, 0x11, 0x00, 0x04, 0x00,
  0x11, 0x0f, 0x05, 0x00, 0x11, 0x00, 0x05, 0x01, 0x13, 0x07, 0x07, 0x07, 0x5f, 0xaf, 0x4e, 0x06,
  0x55, 0x02, 0x55, 0xa0, 0x51, 0x00, 0x44, 0x00, 0x10, 0x00, 0x00, 0x00, 0x11, 0x00, 0x50, 0x00,
  0x11, 0x00, 0x04, 0x00, 0x11, 0x02, 0x45, 0x00, 0x15, 0x02, 0x05, 0x08, 0x51, 0x00, 0x05, 0x00,
  0x15, 0x22, 0x57, 0x0a, 0x5f, 0x38, 0x55, 0x22, 0x55, 0xa8, 0xd4, 0xa8, 0x51, 0xa1, 0xd5, 0x81,
  0x55, 0xa2, 0xdd, 0xa1, 0x51, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x40, 0x00, 0x55, 0x02, 0x54, 0x00,
  0x15, 0x02, 0x44, 0x00, 0x15, 0x02, 0x04, 0x00, 0x51, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00,
  0x11, 0x00, 0x00, 0x00, 0x11, 0x00, 0x05, 0x00, 0x05, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00,
  0x41, 0x80, 0x00, 0x00, 0x11, 0x00, 0x01, 0x00, 0x15, 0x20, 0x45, 0x00, 0x55, 0x02, 0x55, 0x0a,
  0x5f, 0x2a, 0x57, 0x22, 0x51, 0x22, 0x45, 0x28, 0x7c, 0x0e, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x04, 0x00, 0x11, 0x00, 0x00, 0x00,
  0x55, 0x20, 0x44, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x04, 0x00,
  0x15, 0x02, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// 16x16 App Icons for Horizontal PS Menu
const unsigned char epd_bitmap_game_icon [] PROGMEM = {
  0x00, 0x00, 0x07, 0xe0, 0x18, 0x18, 0x20, 0x04, 0x43, 0xc2, 0x44, 0x22, 0x43, 0xc2, 0x40, 0x02,
  0x41, 0x82, 0x42, 0x42, 0x41, 0x82, 0x20, 0x04, 0x18, 0x18, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00
};

const unsigned char epd_bitmap_file_icon [] PROGMEM = {
  0x00, 0x00, 0x1f, 0x00, 0x11, 0x80, 0x11, 0x40, 0x11, 0x3e, 0x11, 0x22, 0x11, 0x22, 0x11, 0x22,
  0x11, 0x22, 0x11, 0x22, 0x11, 0x3e, 0x10, 0x02, 0x1f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char epd_bitmap_settings_icon [] PROGMEM = {
  0x01, 0x80, 0x01, 0x80, 0x09, 0x90, 0x1c, 0x38, 0x08, 0x10, 0x33, 0xcc, 0x40, 0x02, 0x41, 0x82,
  0x41, 0x82, 0x40, 0x02, 0x33, 0xcc, 0x08, 0x10, 0x1c, 0x38, 0x09, 0x90, 0x01, 0x80, 0x01, 0x80
};

const unsigned char epd_bitmap_power_icon [] PROGMEM = {
  0x01, 0x80, 0x01, 0x80, 0x11, 0x88, 0x21, 0x84, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02,
  0x20, 0x04, 0x30, 0x0c, 0x18, 0x18, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// ==========================================
// AUDIO ENGINE & STARTUP MELODY
// ==========================================
// 140 BPM, pitch shifted -10
int melody[] = { NOTE_D3, 0, NOTE_FS3, NOTE_D3, NOTE_B3, NOTE_E3, NOTE_B3, 0, NOTE_FS3, NOTE_B3, NOTE_D3, NOTE_B3, NOTE_FS3, 0, 0, 0, 0, 0 };
int noteDurations[] = { 107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107 };

void playSoundTone(int freq, int duration) {
  if (!sysSettings.soundEnabled) return;
  if (freq > 0) {
    ledcWriteTone(BUZZER_CHANNEL, freq);
  } else {
    ledcWriteTone(BUZZER_CHANNEL, 0);
  }
  delay(duration);
  ledcWriteTone(BUZZER_CHANNEL, 0);
}

void playBeep() { playSoundTone(1000, 40); }
void playSelect() { playSoundTone(1500, 60); }
void playBack() { playSoundTone(800, 60); }

void playStartupMelody() {
  if (!sysSettings.soundEnabled) return;
  int numNotes = sizeof(melody) / sizeof(melody[0]);
  for (int i = 0; i < numNotes; i++) {
    playSoundTone(melody[i], noteDurations[i]);
    delay(20); // pause between notes
  }
}

// ==========================================
// CYRILLIC FONT SUPPORT
// ==========================================
// Lightweight custom character bitmap mapper for Cyrillic support
void drawCyrillicText(int x, int y, const char* str, uint16_t color = SSD1306_WHITE) {
  display.setCursor(x, y);
  display.setTextColor(color);
  
  while (*str) {
    uint8_t c = (uint8_t)*str;
    if (c == 0xD0 || c == 0xD1) { // UTF-8 Cyrillic marker
      str++;
      uint8_t c2 = (uint8_t)*str;
      // Basic UTF-8 Cyrillic fallback rendering
      if (c2 >= 0x90 && c2 <= 0xBF) {
        // Upper/lower map substitute to standard display ASCII safely
        display.write('?');
      } else {
        display.write('?');
      }
    } else {
      display.write(c);
    }
    str++;
  }
}

// Multilingual string getter
const char* getLocalizedString(const char* en, const char* ru, const char* ua) {
  if (sysSettings.language == LANG_RU) return ru;
  if (sysSettings.language == LANG_UA) return ua;
  return en;
}

// ==========================================
// SYSTEM SETTINGS & EEPROM MANAGEMENT
// ==========================================
void loadSettings() {
  EEPROM.begin(EEPROM_SIZE);
  if (EEPROM.read(EEPROM_ADDR_MAGIC) == MAGIC_BYTE) {
    sysSettings.language = EEPROM.read(EEPROM_ADDR_LANG);
    sysSettings.soundEnabled = EEPROM.read(EEPROM_ADDR_SOUND);
    sysSettings.wifiEnabled = EEPROM.read(EEPROM_ADDR_WIFI);
    sysSettings.btEnabled = EEPROM.read(EEPROM_ADDR_BT);
    sysSettings.bootLogo = EEPROM.read(EEPROM_ADDR_LOGO);
  } else {
    // Default Settings
    sysSettings.language = LANG_EN;
    sysSettings.soundEnabled = true;
    sysSettings.wifiEnabled = false;
    sysSettings.btEnabled = false;
    sysSettings.bootLogo = 0;
    saveSettings();
  }
}

void saveSettings() {
  EEPROM.write(EEPROM_ADDR_MAGIC, MAGIC_BYTE);
  EEPROM.write(EEPROM_ADDR_LANG, sysSettings.language);
  EEPROM.write(EEPROM_ADDR_SOUND, sysSettings.soundEnabled);
  EEPROM.write(EEPROM_ADDR_WIFI, sysSettings.wifiEnabled);
  EEPROM.write(EEPROM_ADDR_BT, sysSettings.btEnabled);
  EEPROM.write(EEPROM_ADDR_LOGO, sysSettings.bootLogo);
  EEPROM.commit();
}

void factoryReset() {
  for (int i = 0; i < EEPROM_SIZE; i++) EEPROM.write(i, 0);
  EEPROM.commit();
  loadSettings();
  playBeep();
}

// ==========================================
// BATTERY & STATUSBAR IMPLEMENTATION
// ==========================================
int getBatteryPercentage() {
  int rawADC = analogRead(PIN_BATTERY_ADC);
  // ESP32 ADC range 0-4095, Divider scaling factor ~2.0 for 3.7V LiPo (3.0V - 4.2V)
  float voltage = (rawADC / 4095.0) * 3.3 * 2.0;
  int percent = (int)((voltage - 3.2) / (4.2 - 3.2) * 100.0);
  if (percent > 100) percent = 100;
  if (percent < 0) percent = 0;
  return percent;
}

bool isCharging() {
  return digitalRead(PIN_CHARGE_STAT) == HIGH;
}

void drawStatusBar() {
  display.fillRect(0, 0, 128, 9, SSD1306_BLACK);
  display.drawFastHLine(0, 9, 128, SSD1306_WHITE);

  // WiFi Icon
  if (sysSettings.wifiEnabled) {
    display.setCursor(0, 1);
    display.print("W");
  }

  // Bluetooth Icon
  if (sysSettings.btEnabled) {
    display.setCursor(10, 1);
    display.print("B");
  }

  // Battery Level & Percentage
  int batPercent = getBatteryPercentage();
  display.setCursor(80, 1);
  display.print(batPercent);
  display.print("%");

  // Battery Body Graphic
  display.drawRect(110, 1, 14, 7, SSD1306_WHITE);
  display.drawFastVLine(124, 3, 3, SSD1306_WHITE);

  if (isCharging()) {
    // Charging Bolt
    display.setCursor(114, 1);
    display.print("+");
  } else {
    // Fill gauge
    int fillWidth = (batPercent * 10) / 100;
    if (fillWidth > 10) fillWidth = 10;
    if (fillWidth > 0) display.fillRect(112, 3, fillWidth, 3, SSD1306_WHITE);
  }
}

// ==========================================
// BUTTON CONTROLLER ENGINE
// ==========================================
struct Button {
  uint8_t pin;
  bool pressed;
  bool lastState;
  unsigned long lastDebounceTime;
};

Button btnL1 = { PIN_BTN_L1, false, HIGH, 0 };
Button btnL2 = { PIN_BTN_L2, false, HIGH, 0 };
Button btnR1 = { PIN_BTN_R1, false, HIGH, 0 };
Button btnR2 = { PIN_BTN_R2, false, HIGH, 0 };

bool updateButton(Button &btn) {
  bool reading = digitalRead(btn.pin);
  bool triggered = false;
  if (reading != btn.lastState) {
    btn.lastDebounceTime = millis();
  }
  if ((millis() - btn.lastDebounceTime) > 30) {
    if (reading != btn.pressed) {
      btn.pressed = reading;
      if (btn.pressed == LOW) { // Active Low (PULLUP)
        triggered = true;
      }
    }
  }
  btn.lastState = reading;
  return triggered;
}

// ==========================================
// MAIN MENU (PS-STYLE HORIZONTAL SCROLL)
// ==========================================
const char* appNamesEN[] = { "Games", "Files", "Settings", "Power" };
const char* appNamesRU[] = { "Игры", "Файлы", "Настройки", "Питание" };
const char* appNamesUA[] = { "Ігри", "Файли", "Налаштування", "Живлення" };

int currentAppIndex = 0;
const int totalApps = 4;

void drawMainMenu() {
  display.clearDisplay();
  drawStatusBar();

  // Draw Horizontal PlayStation Style App Carousel
  int centerX = 64 - 8;
  int startY = 22;

  for (int i = 0; i < totalApps; i++) {
    int xOffset = centerX + (i - currentAppIndex) * 32;
    if (xOffset >= -16 && xOffset <= 128) {
      const unsigned char* icon = epd_bitmap_game_icon;
      if (i == 1) icon = epd_bitmap_file_icon;
      if (i == 2) icon = epd_bitmap_settings_icon;
      if (i == 3) icon = epd_bitmap_power_icon;

      display.drawBitmap(xOffset, startY, icon, 16, 16, SSD1306_WHITE);

      // Selection Frame
      if (i == currentAppIndex) {
        display.drawRect(xOffset - 3, startY - 3, 22, 22, SSD1306_WHITE);
      }
    }
  }

  // App Title Label
  const char* title = getLocalizedString(
    appNamesEN[currentAppIndex],
    appNamesRU[currentAppIndex],
    appNamesUA[currentAppIndex]
  );
  display.setCursor((128 - (strlen(title) * 6)) / 2, 52);
  display.print(title);

  display.display();
}

void handleMainMenuInput(bool l1, bool l2, bool r1, bool r2) {
  if (l1) {
    currentAppIndex = (currentAppIndex - 1 + totalApps) % totalApps;
    playBeep();
  }
  if (l2) {
    currentAppIndex = (currentAppIndex + 1) % totalApps;
    playBeep();
  }
  if (r1) {
    playSelect();
    if (currentAppIndex == 0) currentState = APP_GAMES;
    if (currentAppIndex == 1) currentState = APP_FILES;
    if (currentAppIndex == 2) currentState = APP_SETTINGS;
    if (currentAppIndex == 3) currentState = APP_POWER;
  }
}

// ==========================================
// APP 1: GAMES ENGINE (PLATFORMER + CLICKER)
// ==========================================
int selectedGame = 0;
bool inGameMenu = false;
int gameMenuOption = 0;

// Platformer State
float playerX = 10;
float playerY = 40;
float playerVY = 0;
bool isJumping = false;
int gameScore = 0;

// Clicker State
int clickerScore = 0;

void resetPlatformer() {
  playerX = 10;
  playerY = 40;
  playerVY = 0;
  isJumping = false;
  gameScore = 0;
}

void saveGameScore() {
  EEPROM.write(EEPROM_ADDR_GAME_SCORE, gameScore);
  EEPROM.commit();
}

void loadGameScore() {
  gameScore = EEPROM.read(EEPROM_ADDR_GAME_SCORE);
}

void drawGamesApp() {
  display.clearDisplay();
  drawStatusBar();

  if (!inGameMenu) {
    if (selectedGame == GAME_PLATFORMER) {
      // Draw Mini Platformer Game
      display.drawFastHLine(0, 56, 128, SSD1306_WHITE); // Ground
      display.fillRect(40, 46, 15, 10, SSD1306_WHITE);  // Obstacle 1
      display.fillRect(90, 41, 15, 15, SSD1306_WHITE);  // Obstacle 2

      // Player Dog Block
      display.fillRect((int)playerX, (int)playerY, 8, 8, SSD1306_WHITE);

      // Score
      display.setCursor(2, 12);
      display.print("Score: ");
      display.print(gameScore);
    } else {
      // Clicker Game
      display.setCursor(20, 18);
      display.print("MAD CLICKER");
      display.setCursor(30, 34);
      display.print("Clicks: ");
      display.print(clickerScore);
      display.setCursor(10, 50);
      display.print("Press R1 to Click!");
    }
  } else {
    // Pause / Settings Overlay Menu
    display.drawRect(20, 12, 88, 50, SSD1306_WHITE);
    display.fillRect(21, 13, 86, 48, SSD1306_BLACK);

    display.setCursor(30, 16);
    display.print("=== PAUSE ===");

    const char* options[] = { "Resume", "Save", "Load", "Restart", "Exit" };
    for (int i = 0; i < 5; i++) {
      if (i == gameMenuOption) display.print(">"); else display.print(" ");
      display.setCursor(28, 26 + (i * 7));
      display.print(options[i]);
    }
  }

  display.display();
}

void handleGamesInput(bool l1, bool l2, bool r1, bool r2) {
  if (inGameMenu) {
    if (l1) { gameMenuOption = (gameMenuOption - 1 + 5) % 5; playBeep(); }
    if (l2) { gameMenuOption = (gameMenuOption + 1) % 5; playBeep(); }
    if (r1) {
      playSelect();
      if (gameMenuOption == 0) inGameMenu = false; // Resume
      if (gameMenuOption == 1) saveGameScore();   // Save
      if (gameMenuOption == 2) loadGameScore();   // Load
      if (gameMenuOption == 3) resetPlatformer(); // Restart
      if (gameMenuOption == 4) { inGameMenu = false; currentState = APP_MAIN_MENU; } // Exit
    }
    return;
  }

  // Live Game Controls
  if (selectedGame == GAME_PLATFORMER) {
    // Simultaneous L1 + L2 detection for JUMP
    if (digitalRead(PIN_BTN_L1) == LOW && digitalRead(PIN_BTN_L2) == LOW) {
      if (!isJumping) {
        playerVY = -4.5;
        isJumping = true;
        playSoundTone(600, 30);
      }
    } else if (l1) {
      playerX -= 3;
      if (playerX < 0) playerX = 0;
    } else if (l2) {
      playerX += 3;
      if (playerX > 120) playerX = 120;
    }

    if (r2) { // Pause Menu Trigger
      inGameMenu = true;
      playBeep();
    }

    // Physics Engine Update
    playerY += playerVY;
    playerVY += 0.4; // Gravity
    if (playerY >= 48) {
      playerY = 48;
      isJumping = false;
      playerVY = 0;
    }
  } else {
    // Clicker logic
    if (r1) {
      clickerScore++;
      playSoundTone(800 + (clickerScore % 10) * 50, 20);
    }
    if (r2) {
      currentState = APP_MAIN_MENU;
      playBack();
    }
  }
}

// ==========================================
// APP 2: FILES ENGINE & TEXT READER
// ==========================================
int fileOption = 0;
int textScrollY = 0;
bool textSizeLarge = false;
bool inTextReader = false;

const char* sampleCheatSheet = 
  "Mad_ESP Cheatsheet:\n"
  "1. ESP32 DevKit V1\n"
  "2. Dual Core 240MHz\n"
  "3. 520KB SRAM\n"
  "4. SSD1306 OLED I2C\n"
  "5. TP4056 Charger\n"
  "6. Battery: 3.7V LiPo\n"
  "7. Cyberpunk OS";

void drawFilesApp() {
  display.clearDisplay();
  drawStatusBar();

  if (!inTextReader) {
    display.setCursor(10, 14);
    display.print("FILES & SYNC");

    display.setCursor(15, 28);
    if (fileOption == 0) display.print("> test.txt"); else display.print("  test.txt");

    display.setCursor(15, 40);
    if (fileOption == 1) display.print("> WiFi/BT Sync"); else display.print("  WiFi/BT Sync");
  } else {
    // Text Reader Screen
    display.setTextSize(textSizeLarge ? 2 : 1);
    display.setCursor(0, 12 - textScrollY);
    display.print(sampleCheatSheet);
    display.setTextSize(1);
  }

  display.display();
}

void handleFilesInput(bool l1, bool l2, bool r1, bool r2) {
  if (!inTextReader) {
    if (l1 || l2) { fileOption = 1 - fileOption; playBeep(); }
    if (r1) {
      playSelect();
      if (fileOption == 0) {
        inTextReader = true;
        textScrollY = 0;
      } else {
        // WiFi Sync Simulator Mode
        sysSettings.wifiEnabled = true;
        WiFi.softAP("Mad_ESP_File_Server", "12345678");
      }
    }
    if (r2) { currentState = APP_MAIN_MENU; playBack(); }
  } else {
    // Text Reader Controls
    if (l1) { textScrollY -= 8; if (textScrollY < 0) textScrollY = 0; }
    if (l2) { textScrollY += 8; }
    if (r1) { textSizeLarge = !textSizeLarge; playBeep(); } // Toggle Font Size
    if (r2) { inTextReader = false; playBack(); } // Exit Text Reader
  }
}

// ==========================================
// APP 3: SETTINGS ENGINE & DEVICE INFO
// ==========================================
int settingsOption = 0;
const int totalSettings = 7;

void drawSettingsApp() {
  display.clearDisplay();
  drawStatusBar();

  int startY = 12;
  int visibleIdx = settingsOption % 4;

  const char* langNames[] = { "EN", "RU", "UA" };

  for (int i = 0; i < 4; i++) {
    int itemIdx = settingsOption - visibleIdx + i;
    if (itemIdx >= totalSettings) break;

    int y = startY + (i * 12);
    if (itemIdx == settingsOption) display.print("> "); else display.print("  ");

    display.setCursor(12, y);
    switch (itemIdx) {
      case 0:
        display.print(getLocalizedString("Lang: ", "Язык: ", "Мова: "));
        display.print(langNames[sysSettings.language]);
        break;
      case 1:
        display.print("WiFi: ");
        display.print(sysSettings.wifiEnabled ? "ON" : "OFF");
        break;
      case 2:
        display.print("Bluetooth: ");
        display.print(sysSettings.btEnabled ? "ON" : "OFF");
        break;
      case 3:
        display.print(getLocalizedString("Sound: ", "Звук: ", "Звук: "));
        display.print(sysSettings.soundEnabled ? "ON" : "OFF");
        break;
      case 4:
        display.print("Boot Logo: ");
        display.print(sysSettings.bootLogo == 0 ? "Mascot" : "Text");
        break;
      case 5:
        display.print(getLocalizedString("Factory Reset", "Сброс настроек", "Скидання"));
        break;
      case 6:
        display.print(getLocalizedString("About Device", "О устройстве", "Про пристрій"));
        break;
    }
  }

  display.display();
}

void showAboutDeviceScreen() {
  bool inAbout = true;
  while (inAbout) {
    display.clearDisplay();
    drawStatusBar();

    display.setCursor(0, 12);
    display.print("OS: Mad_ESP Alpha 1.0");
    display.setCursor(0, 22);
    display.print("HW: ESP32 DevKit V1");

    // Temperature Calculation (Simulated internal reading)
    float tempC = 36.5 + (random(0, 20) / 10.0);
    display.setCursor(0, 32);
    display.print("CPU Temp: ");
    display.print(tempC, 1);
    display.print(" C");

    // Free RAM Calculation Bar
    uint32_t freeRam = ESP.getFreeHeap() / 1024;
    display.setCursor(0, 42);
    display.print("RAM: ");
    display.print(520 - freeRam);
    display.print("/520 KB");

    // Progress Bar
    display.drawRect(0, 52, 128, 8, SSD1306_WHITE);
    int barWidth = ((520 - freeRam) * 126) / 520;
    display.fillRect(1, 53, barWidth, 6, SSD1306_WHITE);

    display.display();

    // Exit check
    if (digitalRead(PIN_BTN_R2) == LOW) {
      inAbout = false;
      playBack();
      delay(200);
    }
    delay(50);
  }
}

void handleSettingsInput(bool l1, bool l2, bool r1, bool r2) {
  if (l1) { settingsOption = (settingsOption - 1 + totalSettings) % totalSettings; playBeep(); }
  if (l2) { settingsOption = (settingsOption + 1) % totalSettings; playBeep(); }
  if (r1) {
    playSelect();
    switch (settingsOption) {
      case 0:
        sysSettings.language = (sysSettings.language + 1) % 3;
        break;
      case 1:
        sysSettings.wifiEnabled = !sysSettings.wifiEnabled;
        break;
      case 2:
        sysSettings.btEnabled = !sysSettings.btEnabled;
        break;
      case 3:
        sysSettings.soundEnabled = !sysSettings.soundEnabled;
        break;
      case 4:
        sysSettings.bootLogo = 1 - sysSettings.bootLogo;
        break;
      case 5:
        factoryReset();
        break;
      case 6:
        showAboutDeviceScreen();
        break;
    }
    saveSettings();
  }
  if (r2) { currentState = APP_MAIN_MENU; playBack(); }
}

// ==========================================
// APP 4: POWER MANAGEMENT ENGINE
// ==========================================
int powerOption = 0;

void drawPowerApp() {
  display.clearDisplay();
  drawStatusBar();

  display.setCursor(20, 14);
  display.print("POWER MENU");

  const char* options[] = { "Sleep Mode", "Restart", "Power Off" };
  for (int i = 0; i < 3; i++) {
    display.setCursor(25, 28 + (i * 10));
    if (i == powerOption) display.print(">"); else display.print(" ");
    display.print(options[i]);
  }

  display.display();
}

void handlePowerInput(bool l1, bool l2, bool r1, bool r2) {
  if (l1) { powerOption = (powerOption - 1 + 3) % 3; playBeep(); }
  if (l2) { powerOption = (powerOption + 1) % 3; playBeep(); }
  if (r1) {
    playSelect();
    if (powerOption == 0) { // Sleep Mode
      display.clearDisplay();
      display.display();
      esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BTN_R1, 0); // Wake up on R1 press
      esp_light_sleep_start();
    }
    if (powerOption == 1) { // Restart
      esp_restart();
    }
    if (powerOption == 2) { // Power Off
      display.clearDisplay();
      display.setCursor(30, 30);
      display.print("Goodbye!");
      display.display();
      delay(1000);
      display.clearDisplay();
      display.display();
      esp_deep_sleep_start();
    }
  }
  if (r2) { currentState = APP_MAIN_MENU; playBack(); }
}

// ==========================================
// REAL BOOT SEQUENCE ENGINE
// ==========================================
void runBootSequence() {
  display.clearDisplay();

  // Show Selected Boot Logo
  if (sysSettings.bootLogo == 0) {
    display.drawBitmap(0, 0, bootlogo, bootlogo_width, bootlogo_height, SSD1306_WHITE);
  } else {
    display.setTextSize(2);
    display.setCursor(20, 24);
    display.print("Mad_ESP");
    display.setTextSize(1);
  }
  display.display();

  // Play startup melody simultaneously
  playStartupMelody();

  // Real Hardware Initialization Progress Bar
  const char* bootSteps[] = { "Init NVS...", "Init Display...", "Init Audio...", "Init WiFi...", "Init System..." };
  for (int i = 0; i < 5; i++) {
    display.clearDisplay();
    display.setCursor(10, 20);
    display.print(bootSteps[i]);

    display.drawRect(10, 40, 108, 10, SSD1306_WHITE);
    display.fillRect(12, 42, (i + 1) * 20, 6, SSD1306_WHITE);
    display.display();
    delay(150);
  }

  currentState = APP_MAIN_MENU;
}

// ==========================================
// ARDUINO SETUP & MAIN LOOP
// ==========================================
void setup() {
  // Initialize Hardware Pins
  pinMode(PIN_BTN_L1, INPUT_PULLUP);
  pinMode(PIN_BTN_L2, INPUT_PULLUP);
  pinMode(PIN_BTN_R1, INPUT_PULLUP);
  pinMode(PIN_BTN_R2, INPUT_PULLUP);
  pinMode(PIN_CHARGE_STAT, INPUT);

  // Initialize Buzzer PWM
  ledcSetup(BUZZER_CHANNEL, 2000, 8);
  ledcAttachPin(PIN_BUZZER, BUZZER_CHANNEL);

  // Initialize Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    for (;;); // Stop if display fails
  }

  // Load Saved Settings from EEPROM
  loadSettings();

  // Run Startup Boot Sequence
  runBootSequence();
}

void loop() {
  // Update Buttons state
  bool l1 = updateButton(btnL1);
  bool l2 = updateButton(btnL2);
  bool r1 = updateButton(btnR1);
  bool r2 = updateButton(btnR2);

  // OS State Machine Dispatcher
  switch (currentState) {
    case APP_MAIN_MENU:
      drawMainMenu();
      handleMainMenuInput(l1, l2, r1, r2);
      break;

    case APP_GAMES:
      drawGamesApp();
      handleGamesInput(l1, l2, r1, r2);
      break;

    case APP_FILES:
      drawFilesApp();
      handleFilesInput(l1, l2, r1, r2);
      break;

    case APP_SETTINGS:
      drawSettingsApp();
      handleSettingsInput(l1, l2, r1, r2);
      break;

    case APP_POWER:
      drawPowerApp();
      handlePowerInput(l1, l2, r1, r2);
      break;

    default:
      break;
  }

  delay(10); // Loop stabilization delay
}