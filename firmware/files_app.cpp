#include "files_app.h"
#include "core.h"
#include "ui.h"
#include "menu.h"
#include "system_utils.h"
#include "transfer.h"

#include <LittleFS.h>
#include <WiFi.h>
#include <algorithm>
#include <vector>

struct TextLine {
  uint32_t start;
  uint16_t bytes;
};

static std::vector<TextLine> layoutText(const char* text, size_t length, int columns) {
  std::vector<TextLine> lines;
  lines.reserve(length / 20 + 4);
  if (!text || !length) return lines;

  size_t pos = 0;
  while (pos < length) {
    if (text[pos] == '\n') {
      lines.push_back({(uint32_t)pos, 0});
      ++pos;
      continue;
    }
    size_t start = pos;
    size_t scan = pos;
    size_t lastSpace = SIZE_MAX;
    int col = 0;
    bool newline = false;

    while (scan < length && col < columns) {
      char c = text[scan];
      if (c == '\r') { ++scan; continue; }
      if (c == '\n') { newline = true; break; }
      if (c == ' ') lastSpace = scan;
      const char* p = text + scan;
      utf8DecodeNext(p);
      size_t next = (size_t)(p - text);
      if (next <= scan) { ++scan; }
      else { scan = next; }
      ++col;
    }

    if (newline) {
      lines.push_back({(uint32_t)start, (uint16_t)(scan - start)});
      pos = scan + 1;
    } else if (scan >= length) {
      lines.push_back({(uint32_t)start, (uint16_t)(scan - start)});
      pos = length;
    } else if (lastSpace != SIZE_MAX && lastSpace > start) {
      lines.push_back({(uint32_t)start, (uint16_t)(lastSpace - start)});
      pos = lastSpace + 1;
      while (pos < length && text[pos] == ' ') ++pos;
    } else {
      lines.push_back({(uint32_t)start, (uint16_t)(scan - start)});
      pos = scan;
    }
  }
  return lines;
}

static void drawTextLine(const char* content, const TextLine& line, int x, int y,
                         uint8_t fontIndex, uint8_t color) {
  ui::textN(fontIndex, x, y, content + line.start, line.bytes, color);
}

static void runTextViewer(const char* filename, char* content, size_t length) {
  bool large = false;
  int page = 0;
  std::vector<TextLine> lines = layoutText(content, length, 20);
  int rows = 4;

  for (;;) {
    systemTick();
    int step = input.navStep();
    int pageCount = lines.empty() ? 1 : ((int)lines.size() + rows - 1) / rows;
    if (page >= pageCount) page = pageCount - 1;
    if (page < 0) page = 0;

    if (step < 0 && page > 0) {
      --page;
      audio.playEffect(SoundEffect::NAV);
    } else if (step > 0 && page + 1 < pageCount) {
      ++page;
      audio.playEffect(SoundEffect::NAV);
    }

    if (input.pressed(BTN_R1)) {
      size_t firstLine = (size_t)page * rows;
      large = !large;
      rows = large ? 3 : 4;
      lines = layoutText(content, length, large ? 15 : 20);
      page = (int)(firstLine / rows);
      if (page >= (int)((lines.size() + rows - 1) / rows)) {
        page = (int)((lines.size() + rows - 1) / rows) - 1;
      }
      if (page < 0) page = 0;
      audio.playEffect(SoundEffect::SELECT);
    }

    if (input.pressed(BTN_R2)) {
      audio.playEffect(SoundEffect::BACK);
      return;
    }

    pageCount = lines.empty() ? 1 : ((int)lines.size() + rows - 1) / rows;
    ui::clear();
    ui::topBar(filename);
    uint8_t font = large ? FONT_IDX_LARGE : FONT_IDX_SMALL;
    int lineHeight = large ? 16 : 12;
    int startY = 13;
    for (int r = 0; r < rows; ++r) {
      int index = page * rows + r;
      if (index >= (int)lines.size()) break;
      drawTextLine(content, lines[index], 3, startY + r * lineHeight, font, 1);
    }
    if (pageCount > 1) ui::scrollbar(126, 13, 50, pageCount, 1, page);
    ui::flush();
    delay(5);
  }
}

static void openFile(const String& name) {
  String path = name;
  if (!path.startsWith("/")) path = "/" + path;
  File file = LittleFS.open(path, FILE_READ);
  if (!file) {
    infoRun(T(APP_FILES), T(FILE_OPEN_FAIL), name.c_str());
    return;
  }
  size_t size = file.size();
  if (size == 0) {
    file.close();
    infoRun(T(APP_FILES), T(FILE_EMPTY), name.c_str());
    return;
  }
  if (size > MAX_TEXT_FILE_BYTES) {
    file.close();
    infoRun(T(APP_FILES), T(FILE_TOO_BIG), name.c_str());
    return;
  }

  char* data = (char*)malloc(size + 1);
  if (!data) {
    file.close();
    infoRun(T(APP_FILES), T(FILE_OPEN_FAIL), name.c_str());
    return;
  }
  size_t got = file.read((uint8_t*)data, size);
  file.close();
  if (got != size) {
    free(data);
    infoRun(T(APP_FILES), T(FILE_OPEN_FAIL), name.c_str());
    return;
  }
  data[size] = 0;
  if (memchr(data, 0, size) != nullptr) {
    free(data);
    infoRun(T(APP_FILES), T(NOT_TEXT_FILE), name.c_str());
    return;
  }

  for (size_t i = 0; i < size; ++i) {
    if (data[i] == '\t') data[i] = ' ';
  }

  size_t offset = 0;
  if (size >= 3 && (uint8_t)data[0] == 0xEF && (uint8_t)data[1] == 0xBB &&
      (uint8_t)data[2] == 0xBF) {
    offset = 3;
  }

  // Normalise Windows CRLF files so the CR code point does not render as '?'.
  size_t contentLength = size - offset;
  size_t compacted = 0;
  for (size_t i = 0; i < contentLength; ++i) {
    char c = data[offset + i];
    if (c != '\r') data[offset + compacted++] = c;
  }
  contentLength = compacted;
  data[offset + contentLength] = 0;

  runTextViewer(name.c_str(), data + offset, contentLength);
  free(data);
}

static void runFileBrowser() {
  std::vector<String> names;
  File root = LittleFS.open("/");
  if (root) {
    File file = root.openNextFile();
    while (file) {
      if (!file.isDirectory()) {
        String name = file.name();
        while (name.startsWith("/")) name.remove(0, 1);
        names.push_back(name);
      }
      file = root.openNextFile();
    }
  }
  std::sort(names.begin(), names.end());

  if (names.empty()) {
    infoRun(T(APP_FILES), T(NO_FILES), T(FILES_WIFI));
    return;
  }

  int selected = 0;
  int first = 0;
  constexpr int visible = 4;
  for (;;) {
    systemTick();
    int step = input.navStep();
    if (step != 0) {
      selected = (selected + step + (int)names.size()) % (int)names.size();
      if (selected < first) first = selected;
      if (selected >= first + visible) first = selected - visible + 1;
      audio.playEffect(SoundEffect::NAV);
    }
    if (input.pressed(BTN_R1)) {
      audio.playEffect(SoundEffect::SELECT);
      openFile(names[selected]);
      // Refresh in case the web uploader changed things while reading.
      root = LittleFS.open("/");
      names.clear();
      if (root) {
        File file = root.openNextFile();
        while (file) {
          if (!file.isDirectory()) {
            String name = file.name();
            while (name.startsWith("/")) name.remove(0, 1);
            names.push_back(name);
          }
          file = root.openNextFile();
        }
      }
      std::sort(names.begin(), names.end());
      if (names.empty()) return;
      selected = min(selected, (int)names.size() - 1);
      first = min(first, max(0, (int)names.size() - visible));
      // R2 was used to leave the viewer / dialog. Do not also close the browser.
      input.clearEvents();
      continue;
    }
    if (input.pressed(BTN_R2)) {
      audio.playEffect(SoundEffect::BACK);
      return;
    }

    ui::clear();
    ui::topBar(T(APP_FILES));
    for (int row = 0; row < visible && first + row < (int)names.size(); ++row) {
      int index = first + row;
      int y = 13 + row * ROW_HEIGHT;
      bool sel = index == selected;
      if (sel) {
        gfx.setDrawColor(1);
        gfx.drawBox(0, y, 128, ROW_HEIGHT);
      }
      ui::textEllipsis(FONT_IDX_SMALL, 4, y, names[index].c_str(), sel ? 0 : 1, 123);
      if (sel) {
        gfx.setDrawColor(0);
        gfx.drawBox(0, y + 1, 2, ROW_HEIGHT - 2);
      }
    }
    if (names.size() > visible) {
      ui::scrollbar(126, 13, 48, (int)names.size(), visible, first);
    }
    ui::flush();
    delay(5);
  }
}

static void runWifiUploadScreen() {
  bool error = false;
  for (;;) {
    systemTick();
    if (input.pressed(BTN_R2)) {
      audio.playEffect(SoundEffect::BACK);
      return;
    }
    if (!wifiService.active() && input.pressed(BTN_R1)) {
      if (wifiService.begin()) {
        settings.wifiEnabled = true;
        settingsSave();
        audio.playEffect(SoundEffect::SELECT);
      } else {
        audio.playEffect(SoundEffect::ERROR);
        error = true;
      }
    }

    ui::clear();
    ui::topBar(T(WIFI));
    if (!wifiService.active()) {
      ui::textAligned(FONT_IDX_SMALL, 64, 20, T(WIFI_DISABLED), ui::Align::CENTER, 1, 124);
      if (error) ui::textAligned(FONT_IDX_SMALL, 64, 36, T(WIFI_FAIL), ui::Align::CENTER, 1, 124);
      else ui::textAligned(FONT_IDX_SMALL, 64, 36, T(R1_START), ui::Align::CENTER, 1, 124);
      ui::textAligned(FONT_IDX_SMALL, 64, 52, T(R2_BACK), ui::Align::CENTER, 1, 124);
    } else {
      char line[40];
      snprintf(line, sizeof(line), "SSID: %s", WIFI_AP_SSID);
      ui::text(FONT_IDX_SMALL, 4, 14, line, 1, 124);
      snprintf(line, sizeof(line), "PASS: %s", WIFI_AP_PASSWORD);
      ui::text(FONT_IDX_SMALL, 4, 26, line, 1, 124);
      snprintf(line, sizeof(line), "WEB: %s", WiFi.softAPIP().toString().c_str());
      ui::text(FONT_IDX_SMALL, 4, 38, line, 1, 124);
      char clients[32];
      snprintf(clients, sizeof(clients), "%s %u", T(WIFI_CLIENTS), wifiService.clients());
      ui::text(FONT_IDX_SMALL, 4, 50, clients, 1, 100);
      ui::textAligned(FONT_IDX_SMALL, 124, 50, T(R2_BACK), ui::Align::RIGHT, 1, 124);
    }
    ui::flush();
    delay(10);
  }
}

static void runBleUploadScreen() {
  bool error = false;
  for (;;) {
    systemTick();
    if (input.pressed(BTN_R2)) {
      audio.playEffect(SoundEffect::BACK);
      return;
    }
    if (!bleService.active() && input.pressed(BTN_R1)) {
      if (bleService.begin()) {
        settings.bluetoothEnabled = true;
        settingsSave();
        audio.playEffect(SoundEffect::SELECT);
      } else {
        audio.playEffect(SoundEffect::ERROR);
        error = true;
      }
    }

    ui::clear();
    if (!bleService.active()) {
      ui::topBar(T(BLUETOOTH));
      ui::textAligned(FONT_IDX_SMALL, 64, 20, T(BT_OFF), ui::Align::CENTER, 1, 124);
      if (error) ui::textAligned(FONT_IDX_SMALL, 64, 36, T(BT_FAIL), ui::Align::CENTER, 1, 124);
      else ui::textAligned(FONT_IDX_SMALL, 64, 36, T(R1_START), ui::Align::CENTER, 1, 124);
      ui::textAligned(FONT_IDX_SMALL, 64, 52, T(R2_BACK), ui::Align::CENTER, 1, 124);
    } else {
      ui::topBar(T(BT_NAME));
      ui::text(FONT_IDX_SMALL, 4, 14, T(BT_PROTOCOL_PUT), 1, 124);
      ui::text(FONT_IDX_SMALL, 4, 26, T(BT_PROTOCOL_END), 1, 124);
      if (bleService.receiving()) {
        char recv[32];
        snprintf(recv, sizeof(recv), "%s %lu", T(BT_RECEIVING),
                 (unsigned long)bleService.receivedBytes());
        ui::text(FONT_IDX_SMALL, 4, 38, recv, 1, 90);
      } else {
        ui::text(FONT_IDX_SMALL, 4, 38,
                 bleService.connected() ? T(BT_CONNECTED) : T(BT_WAITING), 1, 104);
      }
      ui::textAligned(FONT_IDX_SMALL, 124, 50, T(R2_BACK), ui::Align::RIGHT, 1, 124);
    }
    ui::flush();
    delay(10);
  }
}

void runFilesApp() {
  for (;;) {
    const char* items[3] = {T(FILES_BROWSE), T(FILES_WIFI), T(FILES_BT)};
    int choice = menuRun(T(APP_FILES), items, 3);
    if (choice < 0) return;
    if (choice == 0) {
      runFileBrowser();
    } else if (choice == 1) {
      runWifiUploadScreen();
    } else {
      runBleUploadScreen();
    }
  }
}
