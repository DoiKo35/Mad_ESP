#include <Arduino.h>
#include <Wire.h>
#include <LittleFS.h>
#include <esp_sleep.h>
#include <esp_system.h>

#include "core.h"
#include "config.h"
#include "ui.h"
#include "boot.h"
#include "apps.h"
#include "games_store.h"
#include "system_utils.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C gfx(U8G2_R0, U8X8_PIN_NONE,
                                          PIN_I2C_SCL, PIN_I2C_SDA);

InputManager input;
AudioManager audio;
BatteryMonitor battery;
WifiUploadService wifiService;
BleUploadService bleService;
GameStore gameStore;

static bool initializeStorage() {
  if (!LittleFS.begin(true)) {
    Serial.println("[FS] LittleFS mount failed");
    return false;
  }
  Serial.printf("[FS] LittleFS %.1f KB used / %.1f KB total\n",
                LittleFS.usedBytes() / 1024.0, LittleFS.totalBytes() / 1024.0);
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(80);
  Serial.println();
  Serial.printf("%s %s - %s\n", MADESP_NAME, MADESP_VERSION, MADESP_DEVICE);
  Serial.printf("Reset reason: %d\n", (int)esp_reset_reason());

  const esp_sleep_wakeup_cause_t wakeCause = esp_sleep_get_wakeup_cause();
  const bool wokeFromDeepSleep = (wakeCause == ESP_SLEEP_WAKEUP_EXT0);
  if (wakeCause != ESP_SLEEP_WAKEUP_UNDEFINED) {
    Serial.printf("Wake cause: %d\n", (int)wakeCause);
  }

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  Wire.setClock(I2C_CLOCK_HZ);

  settingsBegin();
  input.begin();
  audio.begin(settings.soundEnabled);
  battery.begin();
  ui::begin();

  if (!wokeFromDeepSleep) bootStart();
  bootProgress(5);

  initializeStorage();
  bootProgress(20);

  gameStore.begin();
  bootProgress(30);

  bool settingsChanged = false;
  if (settings.wifiEnabled) {
    if (!wifiService.begin()) {
      Serial.println("[NET] Wi-Fi AP failed to start");
      settings.wifiEnabled = false;
      settingsChanged = true;
    }
  }
  bootProgress(55);

  if (settings.bluetoothEnabled) {
    if (!bleService.begin()) {
      Serial.println("[BT] BLE failed to start");
      settings.bluetoothEnabled = false;
      settingsChanged = true;
    }
  }
  bootProgress(80);

  battery.update();
  bootProgress(100);
  if (settingsChanged) settingsSave();

  if (!wokeFromDeepSleep) bootFinish();
  else {
    // Power-on wake from deep sleep: get to a usable screen quickly.
    ui::clear();
    ui::topBar("MAD_ESP");
    ui::textAligned(FONT_IDX_LARGE, 64, 24, "Mad_ESP", ui::Align::CENTER, 1, 124);
    ui::flush();
    delay(180);
  }

  // Ignore buttons that were already held during power-up or wake-up.
  delay(40);
  input.update();
  input.clearEvents();

  Serial.printf("[MEM] Free heap: %u bytes\n", (unsigned)ESP.getFreeHeap());
}

void loop() {
  runHome();
}
