#include "apps.h"
#include "core.h"
#include "ui.h"
#include "menu.h"
#include "files_app.h"
#include "games_store.h"
#include "system_utils.h"
#include "config.h"

#include <LittleFS.h>
#include <WiFi.h>
#include <cmath>
#include <driver/gpio.h>
#include <driver/rtc_io.h>
#include <esp_sleep.h>

// ============================================================================
//  Home carousel
// ============================================================================
static int wrapApp(int index) {
  int count = APP_TOTAL;
  index %= count;
  if (index < 0) index += count;
  return index;
}

static const char* appName(AppId app) {
  switch (app) {
    case AppId::GAMES: return T(APP_GAMES);
    case AppId::FILES: return T(APP_FILES);
    case AppId::SETTINGS: return T(APP_SETTINGS);
    case AppId::POWER: return T(APP_POWER);
    case AppId::ABOUT: return T(APP_ABOUT);
    default: return "";
  }
}

static void launchApp(AppId app) {
  switch (app) {
    case AppId::GAMES: runGamesApp(); break;
    case AppId::FILES: runFilesApp(); break;
    case AppId::SETTINGS: runSettingsApp(); break;
    case AppId::POWER: runPowerApp(); break;
    case AppId::ABOUT: runAboutApp(); break;
    default: break;
  }
}

static void drawHome(float cursor) {
  ui::clear();
  ui::topBar("MAD_ESP");

  const float half = 2.5f;
  for (int i = 0; i < APP_TOTAL; ++i) {
    float d = (float)i - cursor;
    while (d > half) d -= (float)APP_TOTAL;
    while (d < -half) d += (float)APP_TOTAL;
    if (fabsf(d) > 1.45f) continue;
    int size = 22 - (int)(fabsf(d) * 10.0f);
    if (size < 8) size = 8;
    int centerX = 64 + (int)lroundf(d * 51.0f);
    int centerY = 25;
    int ix = centerX - size / 2;
    int iy = centerY - size / 2;
    ui::appIcon((AppId)i, ix, iy, size);
  }

  // Central selection frame and chevrons.
  gfx.setDrawColor(1);
  gfx.drawRFrame(50, 13, 28, 26, 4);
  ui::chevron(2, 20, true);
  ui::chevron(124, 20, false);

  int titleIndex = wrapApp((int)lroundf(cursor));
  ui::textAligned(FONT_IDX_LARGE, 64, 40, appName((AppId)titleIndex),
                  ui::Align::CENTER, 1, 126);

  // Carousel pager.
  int start = 50;
  for (int i = 0; i < APP_TOTAL; ++i) {
    int x = start + i * 6;
    if (i == titleIndex) gfx.drawBox(x - 1, 58, 4, 3);
    else gfx.drawBox(x, 59, 2, 2);
  }

  ui::flush();
}

void runHome() {
  static int lastSelected = 0;
  float cursor = (float)lastSelected;
  float target = cursor;
  bool forceDraw = true;
  uint32_t nextStatusDraw = 0;

  for (;;) {
    systemTick();
    int step = input.navStep();
    if (step != 0) {
      target += (float)step;
      audio.playEffect(SoundEffect::NAV);
      forceDraw = true;
    }

    if (input.pressed(BTN_R1)) {
      int appIndex = wrapApp((int)lroundf(target));
      lastSelected = appIndex;
      audio.playEffect(SoundEffect::SELECT);
      launchApp((AppId)appIndex);
      cursor = (float)lastSelected;
      target = cursor;
      input.clearEvents();
      forceDraw = true;
    }

    float delta = target - cursor;
    if (fabsf(delta) > 0.002f) {
      cursor += delta * 0.28f;
      forceDraw = true;
    } else if (fabsf(target - cursor) <= 0.002f) {
      int normalized = wrapApp((int)lroundf(target));
      if (fabsf(target - (float)normalized) > 0.1f) {
        cursor = (float)normalized;
        target = cursor;
      }
    }

    uint32_t now = millis();
    if (forceDraw || now >= nextStatusDraw) {
      drawHome(cursor);
      nextStatusDraw = now + 600;
      forceDraw = false;
    }
    delay(8);
  }
}

// ============================================================================
//  About
// ============================================================================
static void drawPageDots(uint8_t page, uint8_t count) {
  int totalWidth = count * 7 - 3;
  int x = (128 - totalWidth) / 2;
  gfx.setDrawColor(1);
  for (uint8_t i = 0; i < count; ++i) {
    if (i == page) gfx.drawBox(x, 62, 5, 2);
    else gfx.drawBox(x + 1, 62, 3, 2);
    x += 7;
  }
}

void runAboutApp() {
  uint8_t page = 0;
  float tempC = 0.0f;
  bool tempValid = false;
  uint32_t tempAt = 0;

  for (;;) {
    systemTick();
    if (millis() - tempAt > 1500) {
      tempAt = millis();
      tempValid = readChipTemperature(tempC);
    }

    if (input.pressed(BTN_L1) || input.pressed(BTN_R1)) {
      page = (page + 1) % 2;
      audio.playEffect(SoundEffect::NAV);
    }
    if (input.pressed(BTN_L2)) {
      page = (page + 1) % 2;
      audio.playEffect(SoundEffect::NAV);
    }
    if (input.pressed(BTN_R2)) {
      audio.playEffect(SoundEffect::BACK);
      return;
    }

    ui::clear();
    ui::topBar(T(APP_ABOUT));
    char buffer[48];
    char buffer2[40];

    if (page == 0) {
      snprintf(buffer, sizeof(buffer), "%s %s", MADESP_NAME, MADESP_VERSION);
      ui::text(FONT_IDX_SMALL, 3, 14, buffer, 1, 125);
      ui::text(FONT_IDX_SMALL, 3, 26, MADESP_DEVICE, 1, 125);

      if (tempValid) snprintf(buffer, sizeof(buffer), "%s %.1fC", T(CPU_TEMP), tempC);
      else snprintf(buffer, sizeof(buffer), "%s N/A", T(CPU_TEMP));
      ui::text(FONT_IDX_SMALL, 3, 38, buffer, 1, 70);

      formatUptimeShort(millis(), buffer2, sizeof(buffer2));
      snprintf(buffer, sizeof(buffer), "%s %s", T(UP), buffer2);
      ui::textAligned(FONT_IDX_SMALL, 125, 38, buffer, ui::Align::RIGHT, 1, 125);

      uint32_t used = usedHeapKB();
      uint32_t total = totalHeapKB();
      snprintf(buffer, sizeof(buffer), "%s %lu/%luK", T(ABOUT_RAM),
               (unsigned long)used, (unsigned long)total);
      ui::text(FONT_IDX_SMALL, 3, 50, buffer, 1, 78);
      int ramPct = total ? (int)((used * 100) / total) : 0;
      ui::progressBar(82, 54, 42, 5, ramPct);
      drawPageDots(0, 2);
    } else {
      if (battery.valid()) {
        snprintf(buffer, sizeof(buffer), "BAT %.2fV %uMAH", battery.voltage(),
                 (unsigned)BATTERY_CAPACITY_MAH);
      } else {
        snprintf(buffer, sizeof(buffer), "BAT --");
      }
      ui::text(FONT_IDX_SMALL, 3, 14, buffer, 1, 125);

      snprintf(buffer, sizeof(buffer), "%s %luK", T(ABOUT_HEAP),
               (unsigned long)freeHeapKB());
      ui::text(FONT_IDX_SMALL, 3, 26, buffer, 1, 125);

      snprintf(buffer, sizeof(buffer), "%s %luK", T(ABOUT_FLASH),
               (unsigned long)(ESP.getFlashChipSize() / 1024));
      ui::text(FONT_IDX_SMALL, 3, 38, buffer, 1, 125);

      if (wifiService.active()) {
        snprintf(buffer, sizeof(buffer), "%s %s", T(ABOUT_IP),
                 WiFi.softAPIP().toString().c_str());
      } else {
        snprintf(buffer, sizeof(buffer), "%s %s", T(ABOUT_IP), T(OFF));
      }
      ui::text(FONT_IDX_SMALL, 3, 50, buffer, 1, 125);
      drawPageDots(1, 2);
    }

    ui::flush();
    delay(12);
  }
}

// ============================================================================
//  Settings
// ============================================================================
static void showToastMessage(const char* message, uint32_t& until) {
  if (!message) return;
  ui::toast(message);
  until = millis() + 1700;
}

static void factoryResetNow() {
  wifiService.end();
  bleService.end();
  audio.playEffect(SoundEffect::POWER);
  delayWithTick(300);
  settingsReset();
  gameStore.clearAll();
  ESP.restart();
}

static bool confirmFactoryReset() {
  char options[32];
  snprintf(options, sizeof(options), "R1 %s    R2 %s", T(YES), T(NO));
  for (;;) {
    systemTick();
    if (input.pressed(BTN_R1)) {
      audio.playEffect(SoundEffect::SELECT);
      factoryResetNow();
      return true;
    }
    if (input.pressed(BTN_R2)) {
      audio.playEffect(SoundEffect::BACK);
      return false;
    }
    ui::clear();
    ui::topBar(T(RESET));
    ui::textAligned(FONT_IDX_SMALL, 64, 16, T(RESET_WARN1), ui::Align::CENTER, 1, 124);
    ui::textAligned(FONT_IDX_SMALL, 64, 28, T(RESET_WARN2), ui::Align::CENTER, 1, 124);
    ui::textAligned(FONT_IDX_SMALL, 64, 40, T(RESET_FILES_KEPT), ui::Align::CENTER, 1, 124);
    ui::textAligned(FONT_IDX_SMALL, 64, 52, options, ui::Align::CENTER, 1, 124);
    ui::flush();
    delay(8);
  }
}

void runSettingsApp() {
  int selected = 0;
  int first = 0;
  constexpr int visible = 4;
  constexpr int count = 6;
  const char* toastText = nullptr;
  uint32_t toastUntil = 0;

  for (;;) {
    systemTick();
    int step = input.navStep();
    if (step != 0) {
      selected = (selected + step + count) % count;
      if (selected < first) first = selected;
      if (selected >= first + visible) first = selected - visible + 1;
      audio.playEffect(SoundEffect::NAV);
    }

    if (input.pressed(BTN_R1)) {
      switch (selected) {
        case 0:
          settings.language = (Language)(((uint8_t)settings.language + 1) % 3);
          setLanguage(settings.language);
          settingsSave();
          audio.playEffect(SoundEffect::SELECT);
          break;
        case 1: {
          if (wifiService.active()) {
            wifiService.end();
            settings.wifiEnabled = false;
            audio.playEffect(SoundEffect::BACK);
          } else {
            if (wifiService.begin()) {
              settings.wifiEnabled = true;
              audio.playEffect(SoundEffect::SELECT);
            } else {
              settings.wifiEnabled = false;
              audio.playEffect(SoundEffect::ERROR);
              toastText = T(WIFI_FAIL);
              toastUntil = millis() + 1800;
            }
          }
          settingsSave();
          break;
        }
        case 2: {
          if (bleService.active()) {
            bleService.end();
            settings.bluetoothEnabled = false;
            audio.playEffect(SoundEffect::BACK);
          } else {
            if (bleService.begin()) {
              settings.bluetoothEnabled = true;
              audio.playEffect(SoundEffect::SELECT);
            } else {
              settings.bluetoothEnabled = false;
              audio.playEffect(SoundEffect::ERROR);
              toastText = T(BT_FAIL);
              toastUntil = millis() + 1800;
            }
          }
          settingsSave();
          break;
        }
        case 3:
          settings.soundEnabled = !settings.soundEnabled;
          audio.setEnabled(settings.soundEnabled);
          settingsSave();
          if (settings.soundEnabled) audio.playEffect(SoundEffect::SELECT);
          break;
        case 4:
          settings.bootLogo = settings.bootLogo == BootLogoMode::CLASSIC
                                  ? BootLogoMode::TEXT
                                  : BootLogoMode::CLASSIC;
          settingsSave();
          audio.playEffect(SoundEffect::SELECT);
          break;
        case 5:
          audio.playEffect(SoundEffect::SELECT);
          confirmFactoryReset();
          // R2 cancels the confirmation. Do not let that same event close Settings.
          input.clearEvents();
          break;
        default:
          break;
      }
    }

    if (input.pressed(BTN_R2)) {
      audio.playEffect(SoundEffect::BACK);
      return;
    }

    const char* labels[count] = {
      T(LANGUAGE), T(WIFI), T(BLUETOOTH), T(SOUND), T(BOOT_LOGO), T(FACTORY_RESET)
    };
    const char* values[count] = {
      languageName(settings.language),
      wifiService.active() ? T(ON) : T(OFF),
      bleService.active() ? T(ON) : T(OFF),
      settings.soundEnabled ? T(ON) : T(OFF),
      settings.bootLogo == BootLogoMode::CLASSIC ? T(LOGO_CLASSIC) : T(LOGO_TEXT),
      nullptr
    };

    ui::clear();
    ui::topBar(T(APP_SETTINGS));
    for (int row = 0; row < visible && first + row < count; ++row) {
      int index = first + row;
      int y = 13 + row * ROW_HEIGHT;
      bool sel = index == selected;
      if (sel) {
        gfx.setDrawColor(1);
        gfx.drawBox(0, y, 128, ROW_HEIGHT);
      }
      uint8_t color = sel ? 0 : 1;
      int labelRight = values[index] ? 80 : 116;
      ui::textEllipsis(FONT_IDX_SMALL, 3, y, labels[index], color, labelRight);
      if (values[index]) {
        ui::textAligned(FONT_IDX_SMALL, 124, y, values[index], ui::Align::RIGHT, color, 124);
      } else {
        gfx.setDrawColor(color);
        ui::chevron(121, y + 3, false);
      }
      if (sel) {
        gfx.setDrawColor(0);
        gfx.drawBox(0, y + 1, 2, ROW_HEIGHT - 2);
      }
    }
    if (count > visible) ui::scrollbar(126, 13, 48, count, visible, first);
    if (toastText && (int32_t)(millis() - toastUntil) < 0) ui::toast(toastText);
    ui::flush();
    delay(8);
  }
}

// ============================================================================
//  Power
// ============================================================================
static void restartWirelessAfterSleep() {
  if (settings.wifiEnabled && !wifiService.active()) wifiService.begin();
  if (settings.bluetoothEnabled && !bleService.active()) bleService.begin();
}

static bool enterLightSleep() {
  audio.playEffect(SoundEffect::POWER);
  ui::clear();
  ui::textAligned(FONT_IDX_LARGE, 64, 23, T(SLEEPING), ui::Align::CENTER, 1, 124);
  ui::textAligned(FONT_IDX_SMALL, 64, 45, T(POWER_WAKE_HINT), ui::Align::CENTER, 1, 124);
  ui::flush();
  delayWithTick(500);

  audio.stop();
  wifiService.end();
  bleService.end();
  ui::clear();
  ui::flush();
  gfx.setPowerSave(1);

  if (!input.waitAllReleased(SLEEP_RELEASE_TIMEOUT_MS)) {
    gfx.setPowerSave(0);
    restartWirelessAfterSleep();
    return false;
  }

  bool armed = false;
  const int pins[BTN_COUNT] = {PIN_BTN_L1, PIN_BTN_L2, PIN_BTN_R1, PIN_BTN_R2};
  for (uint8_t i = 0; i < BTN_COUNT; ++i) {
    gpio_num_t pin = (gpio_num_t)pins[i];
    if (!rtc_gpio_is_valid_gpio(pin)) continue;
    esp_err_t err = gpio_wakeup_enable(
        pin, BUTTON_ACTIVE_LOW ? GPIO_INTR_LOW_LEVEL : GPIO_INTR_HIGH_LEVEL);
    if (err == ESP_OK) armed = true;
  }
  if (!armed) {
    gfx.setPowerSave(0);
    restartWirelessAfterSleep();
    return false;
  }

  esp_sleep_enable_gpio_wakeup();
  esp_light_sleep_start();

  for (uint8_t i = 0; i < BTN_COUNT; ++i) {
    gpio_num_t pin = (gpio_num_t)pins[i];
    if (rtc_gpio_is_valid_gpio(pin)) gpio_wakeup_disable(pin);
  }

  gfx.setPowerSave(0);
  // Swallow the wake-up press so it does not select an app in the carousel.
  input.update();
  input.clearEvents();
  delay(40);
  input.update();
  input.clearEvents();
  restartWirelessAfterSleep();
  battery.update();
  audio.playEffect(SoundEffect::SELECT);
  return true;
}

static bool enterDeepSleep() {
  gpio_num_t wakePin = (gpio_num_t)PIN_BTN_R1;
  if (!rtc_gpio_is_valid_gpio(wakePin)) return false;

  audio.playEffect(SoundEffect::POWER);
  ui::clear();
  ui::textAligned(FONT_IDX_LARGE, 64, 22, T(POWER_OFF), ui::Align::CENTER, 1, 124);
  ui::textAligned(FONT_IDX_SMALL, 64, 45, T(WAKE_R1), ui::Align::CENTER, 1, 124);
  ui::flush();
  delayWithTick(550);

  audio.stop();
  wifiService.end();
  bleService.end();
  if (!input.waitAllReleased(SLEEP_RELEASE_TIMEOUT_MS)) {
    restartWirelessAfterSleep();
    return false;
  }

  if (BUTTON_ACTIVE_LOW) {
    rtc_gpio_pullup_en(wakePin);
    rtc_gpio_pulldown_dis(wakePin);
  } else {
    rtc_gpio_pulldown_en(wakePin);
    rtc_gpio_pullup_dis(wakePin);
  }
  esp_err_t err = esp_sleep_enable_ext0_wakeup(wakePin, BUTTON_ACTIVE_LOW ? 0 : 1);
  if (err != ESP_OK) {
    restartWirelessAfterSleep();
    return false;
  }
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  ui::clear();
  ui::flush();
  gfx.setPowerSave(1);
  delay(60);
  esp_deep_sleep_start();
  return true;
}

static bool confirmPowerOff() {
  char options[32];
  snprintf(options, sizeof(options), "R1 %s    R2 %s", T(YES), T(NO));
  for (;;) {
    systemTick();
    if (input.pressed(BTN_R1)) {
      audio.playEffect(SoundEffect::SELECT);
      return true;
    }
    if (input.pressed(BTN_R2)) {
      audio.playEffect(SoundEffect::BACK);
      return false;
    }
    ui::clear();
    ui::topBar(T(APP_POWER));
    ui::textAligned(FONT_IDX_LARGE, 64, 22, T(POWER_OFF), ui::Align::CENTER, 1, 124);
    ui::textAligned(FONT_IDX_SMALL, 64, 42, T(WAKE_R1), ui::Align::CENTER, 1, 124);
    ui::textAligned(FONT_IDX_SMALL, 64, 53, options, ui::Align::CENTER, 1, 124);
    ui::flush();
    delay(8);
  }
}

void runPowerApp() {
  for (;;) {
    const char* items[3] = {T(POWER_SLEEP), T(POWER_RESTART), T(POWER_OFF)};
    int choice = menuRun(T(APP_POWER), items, 3);
    if (choice < 0) return;

    if (choice == 0) {
      if (enterLightSleep()) return;
      continue;
    }

    if (choice == 1) {
      audio.playEffect(SoundEffect::POWER);
      ui::clear();
      ui::textAligned(FONT_IDX_LARGE, 64, 23, T(RESTARTING), ui::Align::CENTER, 1, 124);
      ui::flush();
      delayWithTick(500);
      ESP.restart();
    }

    if (choice == 2) {
      if (!confirmPowerOff()) continue;
      if (enterDeepSleep()) return;
      infoRun(T(APP_POWER), "R1 WAKE PIN NOT RTC", "CHECK WIRING / CONFIG");
    }
  }
}
