#include "games_store.h"
#include <Preferences.h>

static Preferences g_games;

void GameStore::begin() {
  if (!_ready) {
    _ready = g_games.begin("madesp_games", false);
  }
  loadMeta();
}

void GameStore::loadMeta() {
  if (!_ready) return;
  _clickerBest = g_games.getULong("click_best", 0);
  _runnerBest = g_games.getULong("run_best", 0);
  _pongWins = g_games.getUShort("pong_wins", 0);
}

void GameStore::saveMeta() {
  if (!_ready) return;
  g_games.putULong("click_best", _clickerBest);
  g_games.putULong("run_best", _runnerBest);
  g_games.putUShort("pong_wins", _pongWins);
}

void GameStore::setClickerBest(uint32_t value) {
  if (value <= _clickerBest) return;
  _clickerBest = value;
  if (_ready) g_games.putULong("click_best", value);
}

void GameStore::setRunnerBest(uint32_t value) {
  if (value <= _runnerBest) return;
  _runnerBest = value;
  if (_ready) g_games.putULong("run_best", value);
}

void GameStore::setPongWins(uint16_t value) {
  if (value <= _pongWins) return;
  _pongWins = value;
  if (_ready) g_games.putUShort("pong_wins", value);
}

bool GameStore::saveRunner(const RunnerSaveData& data) {
  if (!_ready) return false;
  return g_games.putBytes("run_save", &data, sizeof(data)) == sizeof(data);
}

bool GameStore::loadRunner(RunnerSaveData& data) const {
  if (!_ready) return false;
  size_t len = g_games.getBytesLength("run_save");
  if (len != sizeof(data)) return false;
  if (g_games.getBytes("run_save", &data, sizeof(data)) != sizeof(data)) return false;
  return data.version == 1;
}

bool GameStore::hasRunner() const {
  if (!_ready) return false;
  return g_games.isKey("run_save");
}

void GameStore::clearRunner() {
  if (_ready) g_games.remove("run_save");
}

void GameStore::clearAll() {
  if (!_ready) return;
  g_games.clear();
  _clickerBest = 0;
  _runnerBest = 0;
  _pongWins = 0;
  saveMeta();
}
