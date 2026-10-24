#include "settings.h"
#include <Preferences.h>

static Preferences g_prefs;
static bool g_ready = false;

static constexpr uint8_t kMagic = 0x4D;   // 'M'
static constexpr uint8_t kVersion = 1;

SystemSettings settings;

void settingsBegin() {
  if (!g_ready) {
    g_ready = g_prefs.begin("madesp", false);
  }
  if (!g_ready) return;

  uint8_t data[8] = {0};
  size_t len = g_prefs.getBytesLength("cfg");
  if (len == sizeof(data) && g_prefs.getBytes("cfg", data, sizeof(data)) == sizeof(data) &&
      data[0] == kMagic) {
    settings.language = (Language)(data[2] > 2 ? 2 : data[2]);
    settings.wifiEnabled = (data[3] & 0x01) != 0;
    settings.bluetoothEnabled = (data[3] & 0x02) != 0;
    settings.soundEnabled = (data[3] & 0x04) != 0;
    settings.bootLogo = (BootLogoMode)(data[4] > 1 ? 1 : data[4]);
    settings.difficulty = (GameDifficulty)(data[5] > 2 ? 2 : data[5]);
  } else {
    settings = SystemSettings();
  }
  setLanguage(settings.language);
}

void settingsSave() {
  if (!g_ready) return;
  uint8_t data[8] = {0};
  data[0] = kMagic;
  data[1] = kVersion;
  data[2] = (uint8_t)settings.language;
  if (settings.wifiEnabled) data[3] |= 0x01;
  if (settings.bluetoothEnabled) data[3] |= 0x02;
  if (settings.soundEnabled) data[3] |= 0x04;
  data[4] = (uint8_t)settings.bootLogo;
  data[5] = (uint8_t)settings.difficulty;
  g_prefs.putBytes("cfg", data, sizeof(data));
}

void settingsReset() {
  if (g_ready) g_prefs.clear();
  settings = SystemSettings();
  setLanguage(settings.language);
  settingsSave();
}

const char* difficultyName(GameDifficulty difficulty) {
  switch (difficulty) {
    case GameDifficulty::EASY: return T(EASY);
    case GameDifficulty::HARD: return T(HARD);
    default: return T(NORMAL);
  }
}

GameDifficulty cycleDifficulty(GameDifficulty difficulty) {
  switch (difficulty) {
    case GameDifficulty::EASY: return GameDifficulty::NORMAL;
    case GameDifficulty::NORMAL: return GameDifficulty::HARD;
    default: return GameDifficulty::EASY;
  }
}

