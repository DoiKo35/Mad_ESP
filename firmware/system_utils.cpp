#include "system_utils.h"
#include "core.h"

void systemTick() {
  input.update();
  audio.update();
  battery.update();
  wifiService.tick();
  bleService.tick();
}

void delayWithTick(uint32_t ms) {
  uint32_t start = millis();
  while (millis() - start < ms) {
    systemTick();
    delay(4);
  }
}

bool readChipTemperature(float& celsius) {
  if (USE_INTERNAL_TEMP) {
    float t = temperatureRead();
    // Reject obviously bogus values from unsupported silicon.
    if (isnan(t) || t < -40.0f || t > 150.0f) return false;
    celsius = t;
    return true;
  }
  celsius = 0;
  return false;
}
void formatUptimeShort(uint32_t milliseconds, char* out, size_t outSize) {
  uint32_t seconds = milliseconds / 1000;
  uint32_t hours = seconds / 3600;
  uint32_t minutes = (seconds / 60) % 60;
  if (hours < 100) snprintf(out, outSize, "%02lu:%02lu", (unsigned long)hours,
                            (unsigned long)minutes);
  else snprintf(out, outSize, "%luh", (unsigned long)hours);
}

void formatUptimeLong(uint32_t milliseconds, char* out, size_t outSize) {
  uint32_t seconds = milliseconds / 1000;
  uint32_t days = seconds / 86400;
  uint32_t hours = (seconds / 3600) % 24;
  uint32_t minutes = (seconds / 60) % 60;
  if (days > 0) snprintf(out, outSize, "%lud %02lu:%02lu", (unsigned long)days,
                         (unsigned long)hours, (unsigned long)minutes);
  else snprintf(out, outSize, "%02lu:%02lu:%02lu", (unsigned long)hours,
                (unsigned long)minutes, (unsigned long)(seconds % 60));
}

uint32_t totalHeapKB() { return ESP.getHeapSize() / 1024; }
uint32_t freeHeapKB() { return ESP.getFreeHeap() / 1024; }
uint32_t usedHeapKB() {
  uint32_t total = totalHeapKB();
  uint32_t freeKb = freeHeapKB();
  return total > freeKb ? total - freeKb : 0;
}


