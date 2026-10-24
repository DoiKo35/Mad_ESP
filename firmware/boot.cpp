#include "boot.h"
#include "core.h"
#include "ui.h"
#include "config.h"
#include "boot_logo.h"

// Melody generated for Mad_ESP at 140 BPM, pitch -10. The supplied
// espboards.dev note list is already the transposed result.
static const ToneEvent kBootMelody[] = {
  {147, BOOT_MELODY_EVENT_MS}, {0, BOOT_MELODY_EVENT_MS},
  {185, BOOT_MELODY_EVENT_MS}, {147, BOOT_MELODY_EVENT_MS},
  {247, BOOT_MELODY_EVENT_MS}, {165, BOOT_MELODY_EVENT_MS},
  {247, BOOT_MELODY_EVENT_MS}, {0, BOOT_MELODY_EVENT_MS},
  {185, BOOT_MELODY_EVENT_MS}, {247, BOOT_MELODY_EVENT_MS},
  {147, BOOT_MELODY_EVENT_MS}, {247, BOOT_MELODY_EVENT_MS},
  {185, BOOT_MELODY_EVENT_MS}, {0, BOOT_MELODY_EVENT_MS},
  {0, BOOT_MELODY_EVENT_MS}, {0, BOOT_MELODY_EVENT_MS},
  {0, BOOT_MELODY_EVENT_MS}, {0, BOOT_MELODY_EVENT_MS}
};

static bool s_started = false;
static bool s_classic = true;
static bool s_melody = false;
static uint32_t s_startedAt = 0;
static uint8_t s_lastProgress = 255;

static void drawClassicProgress(uint8_t percent) {
  gfx.setDrawColor(0);
  gfx.drawBox(10, 59, 108, 5);
  gfx.setDrawColor(1);
  gfx.drawFrame(10, 59, 108, 4);
  int fill = (104 * percent) / 100;
  if (fill > 0) gfx.drawBox(12, 60, fill, 2);
  gfx.setDrawColor(1);
}

static void drawTextLogo(uint8_t percent) {
  gfx.clearBuffer();
  ui::textScaled(FONT_IDX_LARGE, 8, 7, "Mad_ESP", 1, 2);
  gfx.setDrawColor(1);
  gfx.drawHLine(8, 42, 112);
  ui::textAligned(FONT_IDX_SMALL, 64, 46, "ALPHA 1.0.0", ui::Align::CENTER, 1, 124);
  gfx.drawFrame(10, 59, 108, 4);
  int fill = (104 * percent) / 100;
  if (fill > 0) gfx.drawBox(12, 60, fill, 2);
  gfx.sendBuffer();
}

void bootStart() {
  s_classic = settings.bootLogo == BootLogoMode::CLASSIC;
  s_startedAt = millis();
  s_lastProgress = 255;
  s_melody = false;

  if (s_classic) {
    gfx.clearBuffer();
    gfx.drawXBMP(0, 0, 128, 64, BOOT_LOGO);
    drawClassicProgress(0);
    gfx.sendBuffer();
  } else {
    drawTextLogo(0);
  }

  if (settings.soundEnabled) {
    audio.playSequence(kBootMelody, sizeof(kBootMelody) / sizeof(kBootMelody[0]));
    s_melody = true;
  }
  s_started = true;
}

void bootProgress(uint8_t percent) {
  if (!s_started || percent == s_lastProgress) return;
  s_lastProgress = percent;
  if (s_classic) {
    drawClassicProgress(percent);
  } else {
    drawTextLogo(percent);
  }
  if (s_classic) gfx.sendBuffer();
}

void bootFinish() {
  if (!s_started) return;
  bootProgress(100);

  const uint32_t minimum = 900;
  // Do not cut off the boot melody. While waiting, only audio/battery work.
  for (;;) {
    audio.update();
    battery.update();
    uint32_t elapsed = millis() - s_startedAt;
    if (!audio.busy() && elapsed >= minimum) break;
    if (elapsed > 6500) break;
    delay(5);
  }
  s_started = false;
}
