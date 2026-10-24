#include "audio.h"
#include "config.h"

static const ToneEvent FX_NAV[]   = {{2600, 12}};
static const ToneEvent FX_SELECT[]= {{1900, 42}, {0, 10}, {3000, 55}};
static const ToneEvent FX_BACK[]  = {{1000, 28}, {0, 8}, {700, 36}};
static const ToneEvent FX_ERROR[] = {{420, 85}, {0, 45}, {420, 85}};
static const ToneEvent FX_COIN[]  = {{2400, 28}, {0, 8}, {3200, 48}};
static const ToneEvent FX_JUMP[]  = {{650, 32}, {0, 4}, {1050, 44}};
static const ToneEvent FX_HIT[]   = {{260, 65}, {0, 15}, {170, 95}};
static const ToneEvent FX_BARK[]  = {{760, 36}, {0, 18}, {520, 55}};
static const ToneEvent FX_POWER[] = {{1300, 55}, {0, 20}, {900, 65}, {0, 20}, {560, 110}};

void AudioManager::begin(bool enabled) {
  _enabled = enabled;
  ledcSetup(BUZZER_LEDC_CHANNEL, 2000, 10);
  ledcAttachPin(PIN_BUZZER, BUZZER_LEDC_CHANNEL);
  stopTone();
}

void AudioManager::setEnabled(bool enabled) {
  _enabled = enabled;
  if (!enabled) stop();
}

void AudioManager::startTone(uint16_t frequency) {
  if (_enabled && frequency) ledcWriteTone(BUZZER_LEDC_CHANNEL, frequency);
  else stopTone();
}

void AudioManager::stopTone() {
  ledcWriteTone(BUZZER_LEDC_CHANNEL, 0);
}

void AudioManager::stop() {
  _playing = false;
  _toneOn = false;
  _sequence = nullptr;
  _count = 0;
  _index = 0;
  stopTone();
}

void AudioManager::playTone(uint16_t frequency, uint16_t durationMs) {
  if (!_enabled) return;
  _single = {frequency, durationMs};
  playSequence(&_single, 1);
}

void AudioManager::playSequence(const ToneEvent* sequence, uint8_t count, bool loop) {
  if (!_enabled || !sequence || count == 0) return;
  _sequence = sequence;
  _count = count;
  _index = 0;
  _loop = loop;
  _playing = true;
  _toneOn = false;
  _nextAt = millis();
  stopTone();
}

void AudioManager::playEffect(SoundEffect effect) {
  if (!_enabled) return;
  switch (effect) {
    case SoundEffect::NAV:    playSequence(FX_NAV, sizeof(FX_NAV) / sizeof(FX_NAV[0])); break;
    case SoundEffect::SELECT: playSequence(FX_SELECT, sizeof(FX_SELECT) / sizeof(FX_SELECT[0])); break;
    case SoundEffect::BACK:   playSequence(FX_BACK, sizeof(FX_BACK) / sizeof(FX_BACK[0])); break;
    case SoundEffect::ERROR:  playSequence(FX_ERROR, sizeof(FX_ERROR) / sizeof(FX_ERROR[0])); break;
    case SoundEffect::COIN:   playSequence(FX_COIN, sizeof(FX_COIN) / sizeof(FX_COIN[0])); break;
    case SoundEffect::JUMP:   playSequence(FX_JUMP, sizeof(FX_JUMP) / sizeof(FX_JUMP[0])); break;
    case SoundEffect::HIT:    playSequence(FX_HIT, sizeof(FX_HIT) / sizeof(FX_HIT[0])); break;
    case SoundEffect::BARK:   playSequence(FX_BARK, sizeof(FX_BARK) / sizeof(FX_BARK[0])); break;
    case SoundEffect::POWER:  playSequence(FX_POWER, sizeof(FX_POWER) / sizeof(FX_POWER[0])); break;
  }
}

void AudioManager::update() {
  if (!_playing) return;
  if (!_enabled) { stop(); return; }
  uint32_t now = millis();
  if ((int32_t)(now - _nextAt) < 0) return;

  if (_toneOn) {
    stopTone();
    _toneOn = false;
  }

  if (_index >= _count) {
    if (_loop) {
      _index = 0;
    } else {
      _playing = false;
      _sequence = nullptr;
      return;
    }
  }

  const ToneEvent& event = _sequence[_index++];
  if (event.frequency) {
    startTone(event.frequency);
    _toneOn = true;
  }
  _nextAt = now + event.durationMs;
}
