#include "apps.h"
#include "core.h"
#include "ui.h"
#include "menu.h"
#include "games_store.h"
#include "settings.h"
#include "system_utils.h"
#include "config.h"

#include <cmath>
#include <cstdio>
#include <cstring>

// ============================================================================
//  Shared helpers
// ============================================================================
namespace {

int clampInt(int value, int low, int high) {
  if (value < low) return low;
  if (value > high) return high;
  return value;
}

float clampFloat(float value, float low, float high) {
  if (value < low) return low;
  if (value > high) return high;
  return value;
}

uint32_t clampU32(uint32_t value, uint32_t low, uint32_t high) {
  if (value < low) return low;
  if (value > high) return high;
  return value;
}

void formatCount(uint32_t value, char* out, size_t size) {
  if (value < 1000000UL) {
    snprintf(out, size, "%lu", (unsigned long)value);
  } else if (value < 1000000000UL) {
    snprintf(out, size, "%.1fM", (double)value / 1000000.0);
  } else {
    snprintf(out, size, "%.1fB", (double)value / 1000000000.0);
  }
}

void drawKeyHint(const char* left, const char* right, int y) {
  if (left) ui::text(FONT_IDX_SMALL, 3, y, left, 1, 62);
  if (right) ui::textAligned(FONT_IDX_SMALL, 125, y, right, ui::Align::RIGHT, 1, 125);
}

// A tiny side-view dog used by the runner.  The whole sprite is 9 x 10 px.
void drawRunnerDog(int x, int y, bool facingRight, bool onGround, bool running) {
  gfx.setDrawColor(1);
  if (facingRight) {
    gfx.drawBox(x + 1, y + 3, 6, 5);
    gfx.drawBox(x + 6, y + 1, 3, 4);
    gfx.drawBox(x + 6, y, 2, 2);
    gfx.setDrawColor(0);
    gfx.drawPixel(x + 8, y + 2);
    gfx.setDrawColor(1);
    gfx.drawLine(x + 1, y + 3, x - 1, y + 1);
  } else {
    gfx.drawBox(x + 2, y + 3, 6, 5);
    gfx.drawBox(x, y + 1, 3, 4);
    gfx.drawBox(x + 1, y, 2, 2);
    gfx.setDrawColor(0);
    gfx.drawPixel(x + 1, y + 2);
    gfx.setDrawColor(1);
    gfx.drawLine(x + 7, y + 3, x + 9, y + 1);
  }

  if (onGround && running) {
    if ((millis() / 90) & 1) {
      gfx.drawVLine(x + 2, y + 8, 2);
      gfx.drawVLine(x + 5, y + 8, 1);
    } else {
      gfx.drawVLine(x + 2, y + 8, 1);
      gfx.drawVLine(x + 5, y + 8, 2);
    }
  } else {
    gfx.drawVLine(x + 2, y + 8, 1);
    gfx.drawVLine(x + 5, y + 8, 1);
  }
}

// ============================================================================
//  Clicker
// ============================================================================
constexpr uint32_t kClickMaxLevel = 8;
constexpr uint32_t kAutoMaxLevel = 8;

uint32_t clickUpgradeCost(uint32_t level) {
  if (level >= kClickMaxLevel) return 0xFFFFFFFFu;
  return 25u << (level - 1);
}

uint32_t autoUpgradeCost(uint32_t level) {
  if (level >= kAutoMaxLevel) return 0xFFFFFFFFu;
  return 80u << level;
}

void clickerShop(uint32_t& score, uint32_t& clickLevel, uint32_t& autoLevel) {
  int selected = 0;
  const char* message = nullptr;
  uint32_t messageUntil = 0;

  for (;;) {
    systemTick();
    int step = input.navStep();
    if (step != 0) {
      selected = (selected + 1) & 1;
      audio.playEffect(SoundEffect::NAV);
    }

    if (input.pressed(BTN_R1)) {
      if (selected == 0) {
        uint32_t cost = clickUpgradeCost(clickLevel);
        if (clickLevel >= kClickMaxLevel) {
          audio.playEffect(SoundEffect::ERROR);
          message = T(MAXED);
          messageUntil = millis() + 1400;
        } else if (score >= cost) {
          score -= cost;
          ++clickLevel;
          audio.playEffect(SoundEffect::SELECT);
        } else {
          audio.playEffect(SoundEffect::ERROR);
          message = T(NOT_ENOUGH);
          messageUntil = millis() + 1400;
        }
      } else {
        uint32_t cost = autoUpgradeCost(autoLevel);
        if (autoLevel >= kAutoMaxLevel) {
          audio.playEffect(SoundEffect::ERROR);
          message = T(MAXED);
          messageUntil = millis() + 1400;
        } else if (score >= cost) {
          score -= cost;
          ++autoLevel;
          audio.playEffect(SoundEffect::SELECT);
        } else {
          audio.playEffect(SoundEffect::ERROR);
          message = T(NOT_ENOUGH);
          messageUntil = millis() + 1400;
        }
      }
    }

    if (input.pressed(BTN_R2)) {
      audio.playEffect(SoundEffect::BACK);
      return;
    }

    char bits[32];
    char label[40];
    char costText[24];
    formatCount(score, bits, sizeof(bits));
    snprintf(label, sizeof(label), "%s %s", T(COINS), bits);

    ui::clear();
    ui::topBar(T(SHOP));
    ui::text(FONT_IDX_SMALL, 3, 14, label, 1, 125);

    const char* names[2] = {T(CLICK_POWER), T(AUTO_CLICKER)};
    uint32_t levels[2] = {clickLevel, autoLevel};
    uint32_t costs[2] = {clickUpgradeCost(clickLevel), autoUpgradeCost(autoLevel)};

    for (int row = 0; row < 2; ++row) {
      int y = 26 + row * ROW_HEIGHT;
      bool sel = row == selected;
      if (sel) {
        gfx.setDrawColor(1);
        gfx.drawBox(0, y, 128, ROW_HEIGHT);
      }
      uint8_t color = sel ? 0 : 1;
      snprintf(label, sizeof(label), "%s LV%lu", names[row],
               (unsigned long)levels[row]);
      ui::text(FONT_IDX_SMALL, 3, y, label, color, 94);
      if (levels[row] >= (row == 0 ? kClickMaxLevel : kAutoMaxLevel)) {
        snprintf(costText, sizeof(costText), "%s", T(MAXED));
      } else {
        formatCount(costs[row], bits, sizeof(bits));
        snprintf(costText, sizeof(costText), "%s", bits);
      }
      ui::textAligned(FONT_IDX_SMALL, 125, y, costText, ui::Align::RIGHT, color, 125);
    }

    if (message && (int32_t)(millis() - messageUntil) < 0) {
      ui::textAligned(FONT_IDX_SMALL, 64, 51, message, ui::Align::CENTER, 1, 125);
    } else {
      char left[24];
      char right[24];
      snprintf(left, sizeof(left), "R1 %s", T(BUY));
      snprintf(right, sizeof(right), "R2 %s", T(BACK));
      drawKeyHint(left, right, 51);
    }
    ui::flush();
    delay(8);
  }
}

void runClickerPause(uint32_t& score, uint32_t& peak, uint32_t& clickLevel,
                     uint32_t& autoLevel, bool& exitGame) {
  for (;;) {
    const char* items[3] = {T(CONTINUE_GAME), T(SHOP), T(EXIT)};
    int choice = menuRun(T(PAUSE), items, 3);
    if (choice < 0 || choice == 0) return;
    if (choice == 1) {
      clickerShop(score, clickLevel, autoLevel);
      input.clearEvents();
      continue;
    }
    if (choice == 2) {
      gameStore.setClickerBest(peak);
      exitGame = true;
      return;
    }
  }
}

}  // namespace

void runClickerGame() {
  uint32_t score = 0;
  uint32_t peak = gameStore.clickerBest();
  uint32_t clickLevel = 1;
  uint32_t autoLevel = 0;
  double autoAccumulator = 0.0;
  uint32_t lastClickAt = 0;
  uint32_t lastTime = millis();
  bool exitGame = false;

  while (!exitGame) {
    systemTick();
    uint32_t now = millis();

    if (input.pressed(BTN_R2)) {
      runClickerPause(score, peak, clickLevel, autoLevel, exitGame);
      input.clearEvents();
      lastTime = millis();
      continue;
    }

    if (input.pressed(BTN_L2)) {
      audio.playEffect(SoundEffect::SELECT);
      clickerShop(score, clickLevel, autoLevel);
      input.clearEvents();
      lastTime = millis();
      continue;
    }

    if (input.pressed(BTN_R1)) {
      score += clickLevel;
      if (score > peak) peak = score;
      lastClickAt = now;
      audio.playTone(2500, 18);
    }

    float dt = (now - lastTime) / 1000.0f;
    if (dt < 0.0f) dt = 0.0f;
    if (dt > 0.20f) dt = 0.20f;
    lastTime = now;

    if (autoLevel > 0) {
      autoAccumulator += (double)autoLevel * (double)dt;
      uint32_t gained = (uint32_t)autoAccumulator;
      if (gained > 0) {
        autoAccumulator -= (double)gained;
        score += gained;
        if (score > peak) peak = score;
      }
    }

    char scoreText[24];
    char powerText[24];
    char autoText[24];
    formatCount(score, scoreText, sizeof(scoreText));
    snprintf(powerText, sizeof(powerText), "%s %lu", T(CLICK_POWER),
             (unsigned long)clickLevel);
    snprintf(autoText, sizeof(autoText), "%s %lu/s", T(AUTO),
             (unsigned long)autoLevel);

    ui::clear();
    ui::topBar(T(GAME_CLICKER));

    if (now - lastClickAt < 110) {
      gfx.setDrawColor(1);
      gfx.drawRFrame(1, 13, 126, 19, 2);
    }
    ui::textAligned(FONT_IDX_LARGE, 64, 14, scoreText, ui::Align::CENTER, 1, 127);
    ui::text(FONT_IDX_SMALL, 3, 34, powerText, 1, 62);
    ui::textAligned(FONT_IDX_SMALL, 125, 34, autoText, ui::Align::RIGHT, 1, 125);

    gfx.setDrawColor(1);
    for (int x = 3; x < 125; x += 9) gfx.drawPixel(x, 45);

    char left[32];
    char right[32];
    snprintf(left, sizeof(left), "R1 %s", T(HACK));
    snprintf(right, sizeof(right), "L2 %s", T(SHOP));
    drawKeyHint(left, right, 50);

    ui::flush();
    delay(8);
  }

  gameStore.setClickerBest(peak);
}

// ============================================================================
//  Game settings (shared by runner / pong)
// ============================================================================
void runGameSettings() {
  int selected = 0;

  for (;;) {
    systemTick();
    int step = input.navStep();
    if (step != 0) {
      selected = (selected + 1) & 1;
      audio.playEffect(SoundEffect::NAV);
    }

    if (input.pressed(BTN_R1)) {
      if (selected == 0) {
        settings.difficulty = cycleDifficulty(settings.difficulty);
        settingsSave();
        audio.playEffect(SoundEffect::SELECT);
      } else {
        bool enabling = !settings.soundEnabled;
        if (enabling) {
          settings.soundEnabled = true;
          audio.setEnabled(true);
          audio.playEffect(SoundEffect::SELECT);
        } else {
          audio.playEffect(SoundEffect::BACK);
          settings.soundEnabled = false;
          audio.setEnabled(false);
        }
        settingsSave();
      }
    }

    if (input.pressed(BTN_R2)) {
      audio.playEffect(SoundEffect::BACK);
      return;
    }

    const char* labels[2] = {T(GAME_DIFFICULTY), T(SOUND)};
    const char* values[2] = {difficultyName(settings.difficulty),
                             settings.soundEnabled ? T(ON) : T(OFF)};

    ui::clear();
    ui::topBar(T(APP_SETTINGS));
    for (int row = 0; row < 2; ++row) {
      int y = 13 + row * ROW_HEIGHT;
      bool sel = row == selected;
      if (sel) {
        gfx.setDrawColor(1);
        gfx.drawBox(0, y, 128, ROW_HEIGHT);
      }
      uint8_t color = sel ? 0 : 1;
      ui::text(FONT_IDX_SMALL, 3, y, labels[row], color, 78);
      ui::textAligned(FONT_IDX_SMALL, 125, y, values[row], ui::Align::RIGHT, color, 125);
    }
    char left[24];
    char right[24];
    snprintf(left, sizeof(left), "R1 %s", T(OK));
    snprintf(right, sizeof(right), "R2 %s", T(BACK));
    drawKeyHint(left, right, 50);
    ui::flush();
    delay(8);
  }
}

// ============================================================================
//  Runner
// ============================================================================
namespace {

constexpr float kRunnerGroundY = 58.0f;
constexpr float kRunnerPlayerW = 9.0f;
constexpr float kRunnerPlayerH = 10.0f;
constexpr float kRunnerSpeed = 52.0f;
constexpr float kRunnerGravity = 460.0f;
constexpr float kRunnerJump = -162.0f;
constexpr int kRunnerCoins = 12;
constexpr int kRunnerBugs = 5;
constexpr int kRunnerPlatforms = 4;

struct RunnerState {
  float x;
  float y;
  float vy;
  uint8_t hearts;
  uint8_t difficulty;
  int32_t score;
  uint32_t coins;
  uint16_t level;
  uint16_t collectedMask;
  bool onGround;
  bool facingRight;
};

struct RunnerPlatform {
  float x;
  uint8_t y;
  uint8_t w;
};

struct RunnerBug {
  float x;
  float minX;
  float maxX;
  float dir;
  bool alive;
};

RunnerPlatform g_platforms[kRunnerPlatforms];
RunnerBug g_bugs[kRunnerBugs];
float g_runnerLevelLength = 1100.0f;

void generateRunnerWorld(uint16_t level) {
  g_runnerLevelLength = 980.0f + (float)level * 110.0f;
  if (g_runnerLevelLength > 1800.0f) g_runnerLevelLength = 1800.0f;

  const float platformSpan =
      (g_runnerLevelLength - 300.0f) / (float)kRunnerPlatforms;
  for (int i = 0; i < kRunnerPlatforms; ++i) {
    float wobble = (float)((level * 37 + i * 61) % 41) - 20.0f;
    float px = 180.0f + (float)i * platformSpan + wobble;
    if (px < 60.0f) px = 60.0f;
    if (px > g_runnerLevelLength - 90.0f) px = g_runnerLevelLength - 90.0f;
    g_platforms[i].x = px;
    g_platforms[i].y = (i & 1) ? 33 : 43;
    g_platforms[i].w = (i & 1) ? 34 : 28;
  }

  const float bugSpan =
      (g_runnerLevelLength - 320.0f) / (float)(kRunnerBugs - 1);
  for (int i = 0; i < kRunnerBugs; ++i) {
    float wobble = (float)((level * 53 + i * 47) % 41) - 20.0f;
    float bx = 230.0f + (float)i * bugSpan + wobble;
    if (bx < 140.0f) bx = 140.0f;
    if (bx > g_runnerLevelLength - 70.0f) bx = g_runnerLevelLength - 70.0f;
    RunnerBug& bug = g_bugs[i];
    bug.x = bx;
    bug.minX = bx - 34.0f;
    bug.maxX = bx + 34.0f;
    bug.dir = (i & 1) ? 1.0f : -1.0f;
    bug.alive = true;
  }
}

void runnerCoinPosition(int index, uint16_t level, float& x, float& y) {
  const float spacing = (g_runnerLevelLength - 180.0f) / (float)(kRunnerCoins - 1);
  float wobble = (float)((level * 29 + index * 13) % 21) - 10.0f;
  x = 110.0f + (float)index * spacing + wobble;
  if (x < 60.0f) x = 60.0f;
  if (x > g_runnerLevelLength - 24.0f) x = g_runnerLevelLength - 24.0f;
  y = 48.0f - (float)(index % 3) * 8.0f;
}

void resetRunnerState(RunnerState& state) {
  state.x = 0.0f;
  state.y = kRunnerGroundY - kRunnerPlayerH;
  state.vy = 0.0f;
  state.hearts = 3;
  state.difficulty = (uint8_t)settings.difficulty;
  state.score = 0;
  state.coins = 0;
  state.level = 1;
  state.collectedMask = 0;
  state.onGround = true;
  state.facingRight = true;
  generateRunnerWorld(state.level);
}

float runnerBugBaseSpeed(uint8_t difficulty) {
  if (difficulty == (uint8_t)GameDifficulty::EASY) return 14.0f;
  if (difficulty == (uint8_t)GameDifficulty::HARD) return 29.0f;
  return 21.0f;
}

bool runnerSurfaceAt(float x, float previousBottom, float nextBottom, float& surface) {
  bool found = false;
  float best = 999.0f;

  if (previousBottom <= kRunnerGroundY + 0.01f && nextBottom >= kRunnerGroundY) {
    best = kRunnerGroundY;
    found = true;
  }

  for (int i = 0; i < kRunnerPlatforms; ++i) {
    const RunnerPlatform& p = g_platforms[i];
    if (x + kRunnerPlayerW <= p.x || x >= p.x + p.w) continue;
    float top = (float)p.y;
    if (previousBottom <= top + 0.01f && nextBottom >= top && top < best) {
      best = top;
      found = true;
    }
  }

  if (found) surface = best;
  return found;
}

void updateRunner(RunnerState& state, float dt, uint32_t now,
                  uint32_t& invulnUntil, uint32_t& lastBark, bool& bothPrev) {
  const bool left = input.down(BTN_L1);
  const bool right = input.down(BTN_L2);
  const bool both = left && right;

  if (both && !bothPrev && state.onGround) {
    state.vy = kRunnerJump;
    state.onGround = false;
    audio.playEffect(SoundEffect::JUMP);
  }
  bothPrev = both;

  int direction = 0;
  if (both) {
    direction = state.facingRight ? 1 : -1;
  } else if (left) {
    direction = -1;
    state.facingRight = false;
  } else if (right) {
    direction = 1;
    state.facingRight = true;
  }

  if (direction != 0) {
    state.x += (float)direction * kRunnerSpeed * dt;
    state.x = clampFloat(state.x, 0.0f, g_runnerLevelLength - kRunnerPlayerW);
  }

  state.vy += kRunnerGravity * dt;
  if (state.vy > 360.0f) state.vy = 360.0f;

  float previousBottom = state.y + kRunnerPlayerH;
  state.y += state.vy * dt;
  float nextBottom = state.y + kRunnerPlayerH;
  float surface = 0.0f;
  if (state.vy >= 0.0f && runnerSurfaceAt(state.x, previousBottom, nextBottom, surface)) {
    state.y = surface - kRunnerPlayerH;
    state.vy = 0.0f;
    state.onGround = true;
  } else {
    state.onGround = false;
  }

  // Collect coins.
  float centerX = state.x + kRunnerPlayerW * 0.5f;
  float centerY = state.y + kRunnerPlayerH * 0.5f;
  for (int i = 0; i < kRunnerCoins; ++i) {
    uint16_t bit = (uint16_t)(1u << i);
    if (state.collectedMask & bit) continue;
    float coinX, coinY;
    runnerCoinPosition(i, state.level, coinX, coinY);
    if (fabsf(centerX - coinX) < 7.0f && fabsf(centerY - coinY) < 8.0f) {
      state.collectedMask |= bit;
      state.score += 10;
      ++state.coins;
      audio.playEffect(SoundEffect::COIN);
    }
  }

  // Bark attack.
  if (input.pressed(BTN_R1) && now - lastBark >= 620) {
    lastBark = now;
    audio.playEffect(SoundEffect::BARK);
    for (int i = 0; i < kRunnerBugs; ++i) {
      RunnerBug& bug = g_bugs[i];
      if (!bug.alive) continue;
      if (fabsf((bug.x + 3.0f) - centerX) < 27.0f) {
        bug.alive = false;
        state.score += 15;
      }
    }
  }

  // Bugs patrol and hurt the player.
  float bugSpeed = runnerBugBaseSpeed(state.difficulty) +
                   (float)((state.level > 5) ? 10 : state.level * 2);
  for (int i = 0; i < kRunnerBugs; ++i) {
    RunnerBug& bug = g_bugs[i];
    if (!bug.alive) continue;
    bug.x += bug.dir * bugSpeed * dt;
    if (bug.x < bug.minX) {
      bug.x = bug.minX;
      bug.dir = 1.0f;
    } else if (bug.x > bug.maxX) {
      bug.x = bug.maxX;
      bug.dir = -1.0f;
    }

    if (now >= invulnUntil && state.hearts > 0 &&
        state.x + kRunnerPlayerW > bug.x && state.x < bug.x + 6.0f &&
        state.y + kRunnerPlayerH > 54.0f && state.y < 58.0f) {
      --state.hearts;
      invulnUntil = now + 1400;
      state.x = clampFloat(state.x - 14.0f, 0.0f, g_runnerLevelLength - kRunnerPlayerW);
      state.y = kRunnerGroundY - kRunnerPlayerH;
      state.vy = 0.0f;
      state.onGround = true;
      audio.playEffect(SoundEffect::HIT);
    }
  }
}

void drawRunnerWorld(const RunnerState& state, uint32_t lastBark,
                     uint32_t invulnUntil) {
  char scoreText[24];
  char title[24];
  formatCount((uint32_t)state.score, scoreText, sizeof(scoreText));
  snprintf(title, sizeof(title), "L%u %s", (unsigned)state.level, scoreText);

  ui::clear();
  ui::topBar(title);

  float maxCamera = g_runnerLevelLength - 100.0f;
  if (maxCamera < 0.0f) maxCamera = 0.0f;
  float camera = clampFloat(state.x - 42.0f, 0.0f, maxCamera);

  // Parallax star field.
  gfx.setDrawColor(1);
  int starOffset = (int)(camera * 0.25f);
  for (int i = 0; i < 18; ++i) {
    int sx = (i * 73 - starOffset) % 128;
    if (sx < 0) sx += 128;
    int sy = 16 + (i * 11) % 20;
    gfx.drawPixel(sx, sy);
  }

  // Ground.
  gfx.drawHLine(0, 58, 128);
  gfx.setDrawColor(1);
  for (int i = 0; i < 10; ++i) {
    int gx = (i * 17 - (int)(camera * 0.5f)) % 128;
    if (gx < 0) gx += 128;
    gfx.drawPixel(gx, 61);
  }

  // Platforms.
  for (int i = 0; i < kRunnerPlatforms; ++i) {
    const RunnerPlatform& p = g_platforms[i];
    int sx = (int)lroundf(p.x - camera);
    int ex = sx + p.w;
    if (ex < 0 || sx > 128) continue;
    int left = sx < 0 ? 0 : sx;
    int right = ex > 128 ? 128 : ex;
    gfx.drawHLine(left, p.y, right - left);
    for (int x = left; x < right; x += 6) gfx.drawPixel(x, p.y + 1);
    if (sx >= 0) gfx.drawVLine(sx, p.y, 3);
    if (ex <= 128) gfx.drawVLine(ex - 1, p.y + 1, 3);
  }

  // Portal.
  int portalX = (int)lroundf(g_runnerLevelLength - camera);
  if (portalX > -10 && portalX < 130) {
    gfx.drawFrame(portalX, 34, 8, 24);
    for (int y = 37; y < 58; y += 4) {
      gfx.drawHLine(portalX + 2, y, 4);
    }
  }

  // Coins.
  for (int i = 0; i < kRunnerCoins; ++i) {
    if (state.collectedMask & (uint16_t)(1u << i)) continue;
    float coinX, coinY;
    runnerCoinPosition(i, state.level, coinX, coinY);
    int sx = (int)lroundf(coinX - camera);
    if (sx < -4 || sx > 131) continue;
    int sy = (int)lroundf(coinY);
    gfx.drawCircle(sx, sy, 2);
    gfx.drawPixel(sx, sy);
  }

  // Bugs.
  for (int i = 0; i < kRunnerBugs; ++i) {
    const RunnerBug& bug = g_bugs[i];
    if (!bug.alive) continue;
    int sx = (int)lroundf(bug.x - camera);
    if (sx < -8 || sx > 130) continue;
    gfx.drawFrame(sx, 54, 6, 4);
    gfx.drawPixel(sx + 1, 53);
    gfx.drawPixel(sx + 4, 53);
    gfx.setDrawColor(0);
    gfx.drawPixel(sx + 4, 55);
    gfx.setDrawColor(1);
  }

  // Player.
  bool visible = millis() >= invulnUntil || ((millis() / 90) & 1) == 0;
  if (visible) {
    int px = (int)lroundf(state.x - camera);
    int py = (int)lroundf(state.y);
    drawRunnerDog(px, py, state.facingRight, state.onGround,
                  input.down(BTN_L1) || input.down(BTN_L2));
  }

  // Bark ring.
  if (lastBark != 0 && millis() - lastBark < 320) {
    int radius = 5 + (int)((millis() - lastBark) / 15);
    int cx = (int)lroundf(state.x - camera) + 4;
    int cy = (int)lroundf(state.y) + 5;
    gfx.drawCircle(cx, cy, radius);
  }

  // Small HUD: hearts on the left, coin count on the right.
  for (int i = 0; i < 3; ++i) {
    ui::heart(3 + i * 7, 14, i < state.hearts);
  }
  gfx.drawCircle(94, 16, 2);
  char coinText[16];
  snprintf(coinText, sizeof(coinText), "x%lu", (unsigned long)state.coins);
  ui::text(FONT_IDX_SMALL, 99, 14, coinText, 1, 126);

  ui::flush();
}

bool runRunnerIntro() {
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

    char line1[32];
    char line2[32];
    char line3[48];
    snprintf(line1, sizeof(line1), "L1/L2 %s", T(RUN));
    snprintf(line2, sizeof(line2), "BOTH %s", T(JUMP));
    snprintf(line3, sizeof(line3), "R1 %s   R2 %s", T(BARK), T(PAUSE));

    ui::clear();
    ui::topBar(T(GAME_RUNNER));
    ui::textAligned(FONT_IDX_SMALL, 64, 14, line1, ui::Align::CENTER, 1, 127);
    ui::textAligned(FONT_IDX_SMALL, 64, 26, line2, ui::Align::CENTER, 1, 127);
    ui::textAligned(FONT_IDX_SMALL, 64, 38, line3, ui::Align::CENTER, 1, 127);
    ui::textAligned(FONT_IDX_SMALL, 64, 51, T(R1_START), ui::Align::CENTER, 1, 127);
    ui::flush();
    delay(10);
  }
}

void showRunnerLevelClear(int32_t bonus) {
  char bonusText[24];
  snprintf(bonusText, sizeof(bonusText), "+%ld", (long)bonus);
  ui::clear();
  ui::topBar(T(GAME_RUNNER));
  ui::textAligned(FONT_IDX_LARGE, 64, 19, T(LEVEL_COMPLETE), ui::Align::CENTER, 1, 127);
  ui::textAligned(FONT_IDX_LARGE, 64, 40, bonusText, ui::Align::CENTER, 1, 127);
  ui::flush();
  delayWithTick(900);
}

void saveRunnerState(const RunnerState& state) {
  RunnerSaveData data;
  memset(&data, 0, sizeof(data));
  data.version = 1;
  data.difficulty = state.difficulty;
  data.hearts = state.hearts;
  data.x = state.x;
  data.y = state.y;
  data.velocityY = state.vy;
  data.score = state.score;
  data.coins = state.coins;
  data.level = state.level;
  data.collectedMask = state.collectedMask;
  gameStore.saveRunner(data);
}

bool loadRunnerState(RunnerState& state) {
  RunnerSaveData data;
  if (!gameStore.loadRunner(data)) return false;

  state.level = data.level == 0 ? 1 : data.level;
  generateRunnerWorld(state.level);
  state.x = clampFloat(data.x, 0.0f, g_runnerLevelLength - kRunnerPlayerW);
  state.y = clampFloat(data.y, 13.0f, kRunnerGroundY - kRunnerPlayerH);
  state.vy = clampFloat(data.velocityY, -300.0f, 300.0f);
  state.hearts = (uint8_t)clampInt((int)data.hearts, 1, 3);
  state.difficulty = (uint8_t)clampInt((int)data.difficulty, 0, 2);
  state.score = data.score < 0 ? 0 : data.score;
  state.coins = data.coins;
  state.collectedMask = (uint16_t)(data.collectedMask & 0x0FFFu);
  state.onGround = false;
  state.facingRight = true;
  return true;
}

enum class RunnerPauseAction : uint8_t { RESUME, EXIT };

RunnerPauseAction runRunnerPause(RunnerState& state) {
  for (;;) {
    const char* items[6] = {
        T(CONTINUE_GAME), T(NEW_GAME),   T(SAVE_GAME),
        T(LOAD_GAME),     T(GAME_SETTINGS), T(EXIT_GAME)};
    int choice = menuRun(T(PAUSE), items, 6);
    if (choice < 0 || choice == 0) {
      return RunnerPauseAction::RESUME;
    }

    if (choice == 1) {
      resetRunnerState(state);
      input.clearEvents();
      return RunnerPauseAction::RESUME;
    }

    if (choice == 2) {
      saveRunnerState(state);
      gameStore.setRunnerBest((uint32_t)state.score);
      infoRun(T(PAUSE), T(SAVED));
      input.clearEvents();
      continue;
    }

    if (choice == 3) {
      if (loadRunnerState(state)) {
        infoRun(T(PAUSE), T(LOADED));
      } else {
        infoRun(T(PAUSE), T(NO_SAVE_GAME));
      }
      input.clearEvents();
      return RunnerPauseAction::RESUME;
    }

    if (choice == 4) {
      runGameSettings();
      state.difficulty = (uint8_t)settings.difficulty;
      input.clearEvents();
      continue;
    }

    if (choice == 5) {
      return RunnerPauseAction::EXIT;
    }
  }
}

bool drawRunnerGameOver(RunnerState& state) {
  const uint32_t previousBest = gameStore.runnerBest();
  const bool newRecord = (uint32_t)state.score > previousBest;
  gameStore.setRunnerBest((uint32_t)state.score);
  const uint32_t best =
      newRecord ? (uint32_t)state.score : previousBest;

  char scoreText[24];
  char bestText[24];
  char stats[48];
  char topTitle[32];
  char left[32];
  char right[24];
  formatCount((uint32_t)state.score, scoreText, sizeof(scoreText));
  formatCount(best, bestText, sizeof(bestText));
  snprintf(stats, sizeof(stats), "%s %s", T(SCORE), scoreText);
  if (newRecord) {
    strlcpy(topTitle, T(NEW_RECORD), sizeof(topTitle));
  } else {
    snprintf(topTitle, sizeof(topTitle), "%s %s", T(BEST), bestText);
  }
  snprintf(left, sizeof(left), "R1 %s", T(NEW_GAME));
  snprintf(right, sizeof(right), "R2 %s", T(EXIT));

  for (;;) {
    systemTick();
    if (input.pressed(BTN_R1)) {
      audio.playEffect(SoundEffect::SELECT);
      input.clearEvents();
      return false;
    }
    if (input.pressed(BTN_R2)) {
      audio.playEffect(SoundEffect::BACK);
      input.clearEvents();
      return true;
    }

    ui::clear();
    ui::topBar(topTitle);
    ui::textAligned(FONT_IDX_LARGE, 64, 15, T(GAME_OVER), ui::Align::CENTER, 1, 127);
    ui::textAligned(FONT_IDX_SMALL, 64, 37, stats, ui::Align::CENTER, 1, 127);
    gfx.drawHLine(20, 49, 88);
    drawKeyHint(left, right, 51);
    ui::flush();
    delay(10);
  }
}

}  // namespace

void runRunnerGame() {
  if (!runRunnerIntro()) return;

  RunnerState state;
  resetRunnerState(state);
  bool bothPrev = input.down(BTN_L1) && input.down(BTN_L2);
  input.clearEvents();

  uint32_t invulnUntil = 0;
  uint32_t lastBark = 0;
  uint32_t lastTime = millis();
  bool exitGame = false;

  while (!exitGame) {
    bool gameOver = false;
    while (!gameOver && !exitGame) {
      systemTick();
      uint32_t now = millis();

      if (input.pressed(BTN_R2)) {
        RunnerPauseAction action = runRunnerPause(state);
        input.clearEvents();
        bothPrev = input.down(BTN_L1) && input.down(BTN_L2);
        lastTime = millis();
        if (action == RunnerPauseAction::EXIT) {
          exitGame = true;
          break;
        }
        continue;
      }

      float dt = (now - lastTime) / 1000.0f;
      if (dt < 0.0f) dt = 0.0f;
      if (dt > 0.05f) dt = 0.05f;
      lastTime = now;

      updateRunner(state, dt, now, invulnUntil, lastBark, bothPrev);

      if (state.hearts == 0) {
        gameOver = true;
        break;
      }

      if (state.x + kRunnerPlayerW >= g_runnerLevelLength) {
        int32_t bonus = 200 + (int32_t)state.level * 25;
        state.score += bonus;
        ++state.level;
        if (state.level == 0) state.level = 1;
        state.hearts = (uint8_t)clampInt((int)state.hearts + 1, 1, 3);
        state.x = 0.0f;
        state.y = kRunnerGroundY - kRunnerPlayerH;
        state.vy = 0.0f;
        state.onGround = true;
        state.facingRight = true;
        state.collectedMask = 0;
        generateRunnerWorld(state.level);
        showRunnerLevelClear(bonus);
        lastTime = millis();
        bothPrev = input.down(BTN_L1) && input.down(BTN_L2);
        continue;
      }

      drawRunnerWorld(state, lastBark, invulnUntil);
      delay(7);
    }

    if (exitGame) break;

    gameStore.setRunnerBest((uint32_t)state.score);
    bool leave = drawRunnerGameOver(state);
    if (leave) break;

    resetRunnerState(state);
    invulnUntil = 0;
    lastBark = 0;
    lastTime = millis();
    bothPrev = input.down(BTN_L1) && input.down(BTN_L2);
  }

  gameStore.setRunnerBest((uint32_t)state.score);
}

// ============================================================================
//  Pong
// ============================================================================
namespace {

constexpr float kPongTop = 14.0f;
constexpr float kPongBottom = 61.0f;
constexpr float kPongPaddleW = 2.0f;
constexpr float kPongPaddleH = 10.0f;
constexpr uint8_t kPongWinningScore = 7;

struct PongState {
  float playerY;
  float aiY;
  float ballX;
  float ballY;
  float vx;
  float vy;
  uint8_t playerScore;
  uint8_t aiScore;
  bool serving;
  bool matchOver;
};

float pongPlayerPaddleCenter(const PongState& state) {
  return state.playerY + kPongPaddleH * 0.5f;
}

void resetPongMatch(PongState& state) {
  state.playerY = (kPongTop + kPongBottom) * 0.5f - kPongPaddleH * 0.5f;
  state.aiY = state.playerY;
  state.ballX = 63.0f;
  state.ballY = 37.0f;
  state.vx = 0.0f;
  state.vy = 0.0f;
  state.playerScore = 0;
  state.aiScore = 0;
  state.serving = true;
  state.matchOver = false;
}

void servePongBall(PongState& state) {
  float direction = (random(0, 2) == 0) ? -1.0f : 1.0f;
  state.vx = direction * 74.0f;
  state.vy = (float)random(-34, 35);
  state.serving = false;
  state.ballX = 63.0f;
  state.ballY = 37.0f;
  audio.playEffect(SoundEffect::SELECT);
}

void updatePong(PongState& state, float dt) {
  if (state.serving || state.matchOver) return;

  const bool up = input.down(BTN_L1);
  const bool down = input.down(BTN_L2);
  if (up && !down) state.playerY -= 62.0f * dt;
  if (down && !up) state.playerY += 62.0f * dt;
  state.playerY = clampFloat(state.playerY, kPongTop, kPongBottom - kPongPaddleH);

  float aiSpeed;
  if (settings.difficulty == GameDifficulty::EASY) aiSpeed = 25.0f;
  else if (settings.difficulty == GameDifficulty::HARD) aiSpeed = 48.0f;
  else aiSpeed = 35.0f;

  float target = state.ballY + 1.0f - kPongPaddleH * 0.5f;
  if (state.vx > 0.0f) {
    if (target > state.aiY + 1.0f) state.aiY += aiSpeed * dt;
    else if (target < state.aiY - 1.0f) state.aiY -= aiSpeed * dt;
  } else {
    float center = (kPongTop + kPongBottom) * 0.5f - kPongPaddleH * 0.5f;
    if (state.aiY < center - 1.0f) state.aiY += 18.0f * dt;
    else if (state.aiY > center + 1.0f) state.aiY -= 18.0f * dt;
  }
  state.aiY = clampFloat(state.aiY, kPongTop, kPongBottom - kPongPaddleH);

  state.ballX += state.vx * dt;
  state.ballY += state.vy * dt;

  if (state.ballY <= kPongTop) {
    state.ballY = kPongTop;
    state.vy = fabsf(state.vy);
  } else if (state.ballY + 2.0f >= kPongBottom) {
    state.ballY = kPongBottom - 2.0f;
    state.vy = -fabsf(state.vy);
  }

  // Player paddle.
  if (state.vx < 0.0f && state.ballX <= 7.0f && state.ballX >= 3.0f &&
      state.ballY + 2.0f >= state.playerY &&
      state.ballY <= state.playerY + kPongPaddleH) {
    state.ballX = 7.0f;
    state.vx = fabsf(state.vx) * 1.035f;
    if (state.vx > 128.0f) state.vx = 128.0f;
    float offset = (state.ballY + 1.0f) - pongPlayerPaddleCenter(state);
    state.vy = clampFloat(state.vy + offset * 3.0f, -62.0f, 62.0f);
    audio.playTone(1250, 18);
  }

  // AI paddle.
  if (state.vx > 0.0f && state.ballX + 2.0f >= 121.0f &&
      state.ballX <= 125.0f && state.ballY + 2.0f >= state.aiY &&
      state.ballY <= state.aiY + kPongPaddleH) {
    state.ballX = 119.0f;
    state.vx = -fabsf(state.vx) * 1.035f;
    if (state.vx < -128.0f) state.vx = -128.0f;
    float offset = (state.ballY + 1.0f) - (state.aiY + kPongPaddleH * 0.5f);
    state.vy = clampFloat(state.vy + offset * 2.4f, -62.0f, 62.0f);
    audio.playTone(900, 18);
  }

  if (state.ballX < -4.0f) {
    ++state.aiScore;
    state.serving = true;
    state.ballX = 63.0f;
    state.ballY = 37.0f;
    state.vx = 0.0f;
    state.vy = 0.0f;
    audio.playEffect(SoundEffect::ERROR);
    if (state.aiScore >= kPongWinningScore) state.matchOver = true;
  } else if (state.ballX > 130.0f) {
    ++state.playerScore;
    state.serving = true;
    state.ballX = 63.0f;
    state.ballY = 37.0f;
    state.vx = 0.0f;
    state.vy = 0.0f;
    audio.playEffect(SoundEffect::COIN);
    if (state.playerScore >= kPongWinningScore) state.matchOver = true;
  }
}

void drawPong(const PongState& state) {
  char title[20];
  snprintf(title, sizeof(title), "%s %u:%u", T(GAME_PONG),
           (unsigned)state.playerScore, (unsigned)state.aiScore);
  ui::clear();
  ui::topBar(title);

  for (int y = 15; y < 62; y += 5) gfx.drawPixel(63, y);
  gfx.drawBox(4, (int)lroundf(state.playerY), (int)kPongPaddleW,
              (int)kPongPaddleH);
  gfx.drawBox(121, (int)lroundf(state.aiY), (int)kPongPaddleW,
              (int)kPongPaddleH);
  gfx.drawBox((int)lroundf(state.ballX), (int)lroundf(state.ballY), 2, 2);

  if (state.serving) {
    ui::textAligned(FONT_IDX_SMALL, 64, 45, T(PONG_SERVE), ui::Align::CENTER, 1, 126);
  }
  ui::flush();
}

bool drawPongMatchOver(const PongState& state) {
  const bool playerWon = state.playerScore > state.aiScore;
  if (playerWon) gameStore.setPongWins((uint16_t)(gameStore.pongWins() + 1));

  char scoreLine[32];
  snprintf(scoreLine, sizeof(scoreLine), "%u : %u", (unsigned)state.playerScore,
           (unsigned)state.aiScore);
  char left[32];
  char right[24];
  snprintf(left, sizeof(left), "R1 %s", T(NEW_GAME));
  snprintf(right, sizeof(right), "R2 %s", T(EXIT));

  for (;;) {
    systemTick();
    if (input.pressed(BTN_R1)) {
      audio.playEffect(SoundEffect::SELECT);
      input.clearEvents();
      return false;
    }
    if (input.pressed(BTN_R2)) {
      audio.playEffect(SoundEffect::BACK);
      input.clearEvents();
      return true;
    }
    ui::clear();
    ui::topBar(T(GAME_PONG));
    ui::textAligned(FONT_IDX_LARGE, 64, 17,
                    playerWon ? T(PONG_WIN) : T(PONG_LOSE),
                    ui::Align::CENTER, 1, 127);
    ui::textAligned(FONT_IDX_LARGE, 64, 34, scoreLine, ui::Align::CENTER, 1, 127);
    drawKeyHint(left, right, 51);
    ui::flush();
    delay(10);
  }
}

bool runPongPause(PongState& state) {
  for (;;) {
    const char* items[4] = {T(CONTINUE_GAME), T(NEW_GAME), T(GAME_SETTINGS),
                            T(EXIT_GAME)};
    int choice = menuRun(T(PAUSE), items, 4);
    if (choice < 0 || choice == 0) {
      input.clearEvents();
      return false;
    }
    if (choice == 1) {
      resetPongMatch(state);
      input.clearEvents();
      return false;
    }
    if (choice == 2) {
      runGameSettings();
      input.clearEvents();
      continue;
    }
    if (choice == 3) {
      input.clearEvents();
      return true;
    }
  }
}

}  // namespace

void runPongGame() {
  PongState state;
  resetPongMatch(state);
  uint32_t lastTime = millis();
  bool exitGame = false;

  while (!exitGame) {
    systemTick();
    uint32_t now = millis();

    if (input.pressed(BTN_R2)) {
      bool leave = runPongPause(state);
      if (leave) break;
      lastTime = millis();
      continue;
    }

    if (state.matchOver) {
      bool leave = drawPongMatchOver(state);
      if (leave) break;
      resetPongMatch(state);
      lastTime = millis();
      input.clearEvents();
      continue;
    }

    if (state.serving && input.pressed(BTN_R1)) {
      servePongBall(state);
    }

    float dt = (now - lastTime) / 1000.0f;
    if (dt < 0.0f) dt = 0.0f;
    if (dt > 0.05f) dt = 0.05f;
    lastTime = now;

    updatePong(state, dt);
    drawPong(state);
    delay(8);
  }
}

// ============================================================================
//  Game hub
// ============================================================================
void runGamesApp() {
  for (;;) {
    const char* items[3] = {T(GAME_CLICKER), T(GAME_RUNNER), T(GAME_PONG)};
    int choice = menuRun(T(APP_GAMES), items, 3);
    if (choice < 0) return;
    if (choice == 0) runClickerGame();
    else if (choice == 1) runRunnerGame();
    else if (choice == 2) runPongGame();
  }
}
