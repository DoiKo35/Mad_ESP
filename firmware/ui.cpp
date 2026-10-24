#include "ui.h"
#include "core.h"
#include "fonts.h"

namespace ui {

static const uint8_t kMiniDigits[10][5] = {
  {0b111, 0b101, 0b101, 0b101, 0b111},
  {0b010, 0b110, 0b010, 0b010, 0b111},
  {0b111, 0b001, 0b111, 0b100, 0b111},
  {0b111, 0b001, 0b111, 0b001, 0b111},
  {0b101, 0b101, 0b111, 0b001, 0b001},
  {0b111, 0b100, 0b111, 0b001, 0b111},
  {0b111, 0b100, 0b111, 0b101, 0b111},
  {0b111, 0b001, 0b010, 0b010, 0b010},
  {0b111, 0b101, 0b111, 0b101, 0b111},
  {0b111, 0b101, 0b111, 0b001, 0b111}
};

void begin() {
  gfx.begin();
  gfx.setBusClock(I2C_CLOCK_HZ);
  gfx.setContrast(180);
  gfx.clearBuffer();
  gfx.sendBuffer();
}

void clear() { gfx.clearBuffer(); }
void flush() { gfx.sendBuffer(); }

static const FontData& fontData(uint8_t fontIndex) {
  return fontIndex == FONT_IDX_LARGE ? FONT_LARGE : FONT_SMALL;
}

int textWidth(uint8_t fontIndex, const char* value) {
  return fontTextWidth(fontIndex, value);
}

static void drawGlyph(uint8_t fontIndex, int x, int y, uint16_t cp, uint8_t color,
                      uint8_t scale) {
  const uint8_t* rows = fontGlyph(fontIndex, cp);
  if (!rows) return;
  const FontData& f = fontData(fontIndex);
  gfx.setDrawColor(color);
  for (uint8_t row = 0; row < f.height; ++row) {
    uint8_t bits = rows[row];
    if (!bits) continue;
    for (uint8_t col = 0; col < f.width; ++col) {
      if (bits & (0x80u >> col)) {
        if (scale == 1) {
          gfx.drawPixel(x + col, y + row);
        } else {
          gfx.drawBox(x + col * scale, y + row * scale, scale, scale);
        }
      }
    }
  }
}

int text(uint8_t fontIndex, int x, int y, const char* value, uint8_t color,
         int maxRight) {
  if (!value) return 0;
  const FontData& f = fontData(fontIndex);
  int startX = x;
  const char* p = value;
  while (*p) {
    const char* before = p;
    uint16_t cp = utf8DecodeNext(p);
    if (cp == 0) break;
    if (maxRight < 128 && x + f.width > maxRight) {
      p = before;
      break;
    }
    drawGlyph(fontIndex, x, y, cp, color, 1);
    x += f.width;
  }
  return x - startX;
}

void textScaled(uint8_t fontIndex, int x, int y, const char* value, uint8_t color,
                uint8_t scale) {
  if (!value || scale == 0) return;
  const FontData& f = fontData(fontIndex);
  const char* p = value;
  while (*p) {
    uint16_t cp = utf8DecodeNext(p);
    if (cp == 0) break;
    drawGlyph(fontIndex, x, y, cp, color, scale);
    x += f.width * scale;
  }
}

int textN(uint8_t fontIndex, int x, int y, const char* value, size_t length,
          uint8_t color) {
  if (!value || length == 0) return 0;
  const FontData& f = fontData(fontIndex);
  const char* p = value;
  const char* end = value + length;
  int startX = x;
  while (p < end && *p) {
    uint16_t cp = utf8DecodeNext(p);
    if (cp == 0) break;
    drawGlyph(fontIndex, x, y, cp, color, 1);
    x += f.width;
  }
  return x - startX;
}

int textAligned(uint8_t fontIndex, int x, int y, const char* value, Align align,
                uint8_t color, int maxRight) {
  const FontData& f = fontData(fontIndex);
  int w = fontTextWidth(fontIndex, value);
  if (align == Align::RIGHT) x -= w;
  else if (align == Align::CENTER) x -= w / 2;
  return text(fontIndex, x, y, value, color, maxRight);
}

int textEllipsis(uint8_t fontIndex, int x, int y, const char* value, uint8_t color,
                 int maxRight) {
  if (!value) return 0;
  const FontData& f = fontData(fontIndex);
  int fullWidth = fontTextWidth(fontIndex, value);
  if (x + fullWidth <= maxRight) return text(fontIndex, x, y, value, color, maxRight);

  const int dotWidth = f.width * 3;
  int budget = maxRight - x - dotWidth;
  if (budget < f.width) return 0;
  int chars = budget / f.width;

  int startX = x;
  const char* p = value;
  int drawn = 0;
  while (*p && drawn < chars) {
    uint16_t cp = utf8DecodeNext(p);
    if (cp == 0) break;
    drawGlyph(fontIndex, x, y, cp, color, 1);
    x += f.width;
    ++drawn;
  }
  int dotX = x;
  gfx.setDrawColor(color);
  for (int i = 0; i < 3; ++i) {
    gfx.drawPixel(dotX + i * 2, y + f.height - 2);
    gfx.drawPixel(dotX + i * 2 + 1, y + f.height - 2);
  }
  return x + dotWidth - startX;
}

// --------------------------------------------------------------------------
//  Small status widgets
// --------------------------------------------------------------------------
static int miniCharWidth(char c) { return 3; }

static void drawMiniChar(int x, int y, char c, uint8_t color) {
  const uint8_t* glyph = nullptr;
  uint8_t temp[5] = {0};
  if (c >= '0' && c <= '9') {
    glyph = kMiniDigits[c - '0'];
  } else if (c == '%') {
    static const uint8_t pct[5] = {0b101, 0b001, 0b010, 0b100, 0b101};
    glyph = pct;
  } else if (c == '-') {
    static const uint8_t dash[5] = {0b000, 0b000, 0b111, 0b000, 0b000};
    glyph = dash;
  } else if (c == '?') {
    static const uint8_t q[5] = {0b111, 0b001, 0b011, 0b000, 0b010};
    glyph = q;
  } else {
    (void)temp;
    return;
  }
  gfx.setDrawColor(color);
  for (int row = 0; row < 5; ++row) {
    for (int col = 0; col < 3; ++col) {
      if (glyph[row] & (0b100 >> col)) gfx.drawPixel(x + col, y + row);
    }
  }
}

static void miniTextRight(int right, int y, const char* value, uint8_t color) {
  int w = (int)strlen(value) * 4 - 1;
  int x = right - w;
  for (const char* p = value; *p; ++p) {
    drawMiniChar(x, y, *p, color);
    x += 4;
  }
}

void bolt(int x, int y, int h, uint8_t color) {
  gfx.setDrawColor(color);
  static const uint8_t tiny[5] = {0b001, 0b011, 0b111, 0b010, 0b100};
  if (h <= 5) {
    for (int row = 0; row < 5; ++row) {
      for (int col = 0; col < 3; ++col) {
        if (tiny[row] & (0b100 >> col)) gfx.drawPixel(x + col, y + row);
      }
    }
    return;
  }
  // A slightly larger lightning stroke for game / power UI.
  int mid = y + h / 2;
  gfx.drawLine(x + 3, y, x + 1, mid);
  gfx.drawLine(x + 1, mid, x + 3, mid);
  gfx.drawLine(x + 3, mid, x, y + h - 1);
  gfx.drawLine(x + 1, y + h - 2, x + 4, y + h - 2);
}

void batteryIcon(int x, int y, int percent, bool charging) {
  gfx.setDrawColor(1);
  gfx.drawFrame(x, y, 10, 7);
  gfx.drawBox(x + 10, y + 2, 2, 3);
  int fill = percent;
  if (fill < 0) fill = 0;
  if (fill > 100) fill = 100;
  int innerWidth = (8 * fill + 50) / 100;
  if (innerWidth > 0) gfx.drawBox(x + 1, y + 1, innerWidth, 5);

  if (charging) {
    gfx.setDrawColor(0);
    gfx.drawBox(x + 3, y + 1, 4, 5);
    bolt(x + 3, y + 1, 5, 1);
  } else if (percent >= 0 && percent < 15) {
    // One-pixel warning notch on low charge.
    gfx.setDrawColor(0);
    gfx.drawVLine(x + 5, y + 2, 2);
  }
  gfx.setDrawColor(1);
}

void wifiIcon(int x, int y, bool on) {
  gfx.setDrawColor(1);
  if (on) {
    gfx.drawBox(x + 0, y + 6, 2, 2);
    gfx.drawBox(x + 3, y + 3, 2, 5);
    gfx.drawBox(x + 6, y + 0, 2, 8);
  } else {
    gfx.setDrawColor(1);
    gfx.drawFrame(x + 0, y + 6, 2, 2);
    gfx.drawFrame(x + 3, y + 3, 2, 5);
    gfx.drawFrame(x + 6, y + 0, 2, 8);
  }
  if (!on) {
    gfx.drawLine(x, y + 7, x + 7, y);
  }
}

void btIcon(int x, int y, bool on) {
  gfx.setDrawColor(1);
  // Compact Bluetooth rune.
  gfx.drawVLine(x + 4, y, 8);
  gfx.drawLine(x + 4, y, x + 8, y + 2);
  gfx.drawLine(x + 8, y + 2, x + 4, y + 4);
  gfx.drawLine(x + 4, y + 4, x + 8, y + 6);
  gfx.drawLine(x + 8, y + 6, x + 4, y + 8);
  gfx.drawLine(x + 4, y, x, y + 2);
  gfx.drawLine(x, y + 2, x + 4, y + 4);
  if (!on) gfx.drawLine(x, y + 7, x + 8, y - 1);
}

void batteryStatus() {
  bool charging = battery.charging();
  batteryIcon(91, 2, battery.valid() ? battery.percent() : -1, charging);

  char pct[8];
  if (battery.valid() && battery.percent() >= 0) {
    snprintf(pct, sizeof(pct), "%d%%", battery.percent());
  } else {
    snprintf(pct, sizeof(pct), "--%%");
  }
  miniTextRight(88, 4, pct, 1);
  wifiIcon(104, 2, wifiService.active());
  btIcon(115, 2, bleService.active());
}

void topBar(const char* title) {
  // Keep a clean, low-cost title bar. Right side: battery %, Wi-Fi, Bluetooth.
  textEllipsis(FONT_IDX_SMALL, 2, 0, title, 1, 72);
  batteryStatus();
  gfx.setDrawColor(1);
  gfx.drawHLine(0, 12, 128);
}

void progressBar(int x, int y, int w, int h, int percent) {
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  gfx.setDrawColor(1);
  gfx.drawFrame(x, y, w, h);
  int innerW = w - 2;
  int fill = (innerW * percent + 50) / 100;
  if (fill > 0) gfx.drawBox(x + 1, y + 1, fill, h - 2);
}

void scrollbar(int x, int y, int h, int total, int visible, int first) {
  if (total <= visible || h <= 2) return;
  gfx.setDrawColor(1);
  gfx.drawVLine(x, y, h);
  int thumb = max(3, (h * visible) / total);
  int maxFirst = total - visible;
  int pos = 0;
  if (maxFirst > 0) pos = ((h - thumb) * first) / maxFirst;
  gfx.drawBox(x - 1, y + pos, 3, thumb);
}

void chevron(int x, int y, bool left) {
  gfx.setDrawColor(1);
  if (left) {
    gfx.drawLine(x + 2, y, x, y + 3);
    gfx.drawLine(x, y + 3, x + 2, y + 6);
  } else {
    gfx.drawLine(x, y, x + 2, y + 3);
    gfx.drawLine(x + 2, y + 3, x, y + 6);
  }
}

void heart(int x, int y, bool full) {
  static const uint8_t heartBits[5] = {0b10110, 0b11111, 0b11111, 0b01110, 0b00100};
  gfx.setDrawColor(1);
  for (int row = 0; row < 5; ++row) {
    for (int col = 0; col < 5; ++col) {
      bool bit = heartBits[row] & (0b10000 >> col);
      if (!bit) continue;
      if (full) {
        gfx.drawPixel(x + col, y + row);
      } else {
        // Hollow heart: keep border pixels.
        bool border = row == 0 || row == 4 || col == 0 || col == 4 ||
                      (row == 1 && col != 1 && col != 2) ||
                      (row == 3 && (col == 1 || col == 2 || col == 3));
        if (border) gfx.drawPixel(x + col, y + row);
      }
    }
  }
}

void modal(const char* title) {
  gfx.setDrawColor(0);
  gfx.drawBox(4, 14, 120, 48);
  gfx.setDrawColor(1);
  gfx.drawBox(5, 15, 118, 46);
  gfx.setDrawColor(0);
  gfx.drawFrame(6, 16, 116, 44);
  textAligned(FONT_IDX_SMALL, 64, 20, title, Align::CENTER, 0, 123);
  gfx.setDrawColor(1);
}

void toast(const char* message) {
  if (!message || !*message) return;
  int w = fontTextWidth(FONT_IDX_SMALL, message) + 8;
  if (w > 120) w = 120;
  int x = (128 - w) / 2;
  gfx.setDrawColor(0);
  gfx.drawBox(x, 50, w, 13);
  gfx.setDrawColor(1);
  gfx.drawFrame(x, 50, w, 13);
  textEllipsis(FONT_IDX_SMALL, x + 4, 51, message, 1, x + w - 4);
}

// --------------------------------------------------------------------------
//  App icons for the horizontal home carousel
// --------------------------------------------------------------------------
void appIcon(AppId app, int x, int y, int size) {
  if (size < 8) return;
  auto sx = [x, size](int u) { return x + (u * size) / 24; };
  auto sy = [y, size](int u) { return y + (u * size) / 24; };
  gfx.setDrawColor(1);

  switch (app) {
    case AppId::GAMES: {
      int x0 = sx(2), y0 = sy(6), w = sx(20) - x0, h = sy(18) - y0;
      gfx.drawRFrame(x0, y0, w, h, max(2, size / 7));
      int cx = sx(7), cy = sy(12);
      int arm = max(2, size / 9);
      gfx.drawHLine(cx - arm, cy, arm * 2 + 1);
      gfx.drawVLine(cx, cy - arm, arm * 2 + 1);
      gfx.drawDisc(sx(17), sy(10), max(1, size / 14));
      gfx.drawDisc(sx(17), sy(15), max(1, size / 14));
      break;
    }
    case AppId::FILES: {
      gfx.drawFrame(sx(3), sy(8), sx(20) - sx(3), sy(19) - sy(8));
      gfx.drawBox(sx(3), sy(6), sx(11) - sx(3), sy(9) - sy(6));
      if (size >= 18) {
        gfx.setDrawColor(1);
        gfx.drawHLine(sx(6), sy(13), sx(17) - sx(6));
        gfx.drawHLine(sx(6), sy(16), sx(14) - sx(6));
      }
      break;
    }
    case AppId::SETTINGS: {
      const int ys[3] = {7, 12, 17};
      const int knobs[3] = {15, 8, 18};
      for (int i = 0; i < 3; ++i) {
        gfx.drawHLine(sx(3), sy(ys[i]), sx(21) - sx(3));
        gfx.drawDisc(sx(knobs[i]), sy(ys[i]), max(1, size / 14));
      }
      break;
    }
    case AppId::POWER: {
      gfx.drawFrame(sx(5), sy(7), sx(17) - sx(5), sy(18) - sy(7));
      gfx.drawBox(sx(17), sy(10), max(1, size / 12), max(2, size / 5));
      bolt(sx(9), sy(7), max(5, sy(18) - sy(7)), 1);
      break;
    }
    case AppId::ABOUT: {
      int r = size * 7 / 20;
      gfx.drawCircle(sx(12), sy(12), r);
      gfx.drawDisc(sx(12), sy(8), max(1, size / 16));
      gfx.drawVLine(sx(12), sy(11), sy(17) - sy(11));
      break;
    }
    default:
      break;
  }
}

}  // namespace ui
