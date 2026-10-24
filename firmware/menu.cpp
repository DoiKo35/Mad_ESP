#include "menu.h"
#include "core.h"
#include "ui.h"
#include "system_utils.h"

int menuRun(const char* title, const char* const* items, uint8_t count,
            uint8_t startIndex) {
  if (!items || count == 0) return -1;
  int selected = startIndex < count ? startIndex : 0;
  int first = selected >= 4 ? selected - 3 : 0;
  constexpr int visible = 4;

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
      audio.playEffect(SoundEffect::SELECT);
      return selected;
    }
    if (input.pressed(BTN_R2)) {
      audio.playEffect(SoundEffect::BACK);
      return -1;
    }

    ui::clear();
    ui::topBar(title);
    for (int row = 0; row < visible && first + row < count; ++row) {
      int index = first + row;
      int y = 13 + row * ROW_HEIGHT;
      bool sel = index == selected;
      if (sel) {
        gfx.setDrawColor(1);
        gfx.drawBox(0, y, 128, ROW_HEIGHT);
      }
      ui::textEllipsis(FONT_IDX_SMALL, 4, y, items[index], sel ? 0 : 1, 124);
      if (sel) {
        gfx.setDrawColor(0);
        gfx.drawBox(0, y + 1, 2, ROW_HEIGHT - 2);
        ui::chevron(120, y + 3, false);
      }
    }
    if (count > visible) ui::scrollbar(126, 13, 48, count, visible, first);
    ui::flush();
    delay(5);
  }
}

void infoRun(const char* title, const char* line1, const char* line2,
             const char* line3) {
  for (;;) {
    systemTick();
    if (input.pressed(BTN_R1) || input.pressed(BTN_R2)) {
      audio.playEffect(SoundEffect::BACK);
      return;
    }
    ui::clear();
    ui::topBar(title);
    if (line1) ui::textAligned(FONT_IDX_SMALL, 64, 15, line1, ui::Align::CENTER, 1, 124);
    if (line2) ui::textAligned(FONT_IDX_SMALL, 64, 28, line2, ui::Align::CENTER, 1, 124);
    if (line3) ui::textAligned(FONT_IDX_SMALL, 64, 41, line3, ui::Align::CENTER, 1, 124);
    ui::textAligned(FONT_IDX_SMALL, 64, 52, "R1 / R2", ui::Align::CENTER, 1, 124);
    ui::flush();
    delay(5);
  }
}
