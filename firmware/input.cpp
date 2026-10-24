#include "input.h"
#include "config.h"

static const int kButtonPins[BTN_COUNT] = {
  PIN_BTN_L1, PIN_BTN_L2, PIN_BTN_R1, PIN_BTN_R2
};

static bool readButton(uint8_t pin) {
  bool level = digitalRead(pin) == HIGH;
  return BUTTON_ACTIVE_LOW ? !level : level;
}

void InputManager::begin() {
  const uint8_t mode = BUTTON_ACTIVE_LOW ? INPUT_PULLUP : INPUT_PULLDOWN;
  for (uint8_t i = 0; i < BTN_COUNT; ++i) {
    pinMode(kButtonPins[i], mode);
    _state[i] = ButtonState();
    _state[i].raw = readButton(kButtonPins[i]);
    _state[i].rawChangedAt = millis();
  }
  _now = millis();
}

void InputManager::update() {
  _now = millis();

  for (uint8_t i = 0; i < BTN_COUNT; ++i) {
    ButtonState& s = _state[i];
    s.pressed = false;
    s.released = false;
    s.repeated = false;

    bool raw = readButton(kButtonPins[i]);
    if (raw != s.raw) {
      s.raw = raw;
      s.rawChangedAt = _now;
    }

    if (raw != s.down && (_now - s.rawChangedAt) >= BUTTON_DEBOUNCE_MS) {
      s.down = raw;
      if (down(i)) {
        s.pressed = true;
        s.pressedAt = _now;
        s.lastRepeatAt = _now;
      } else {
        s.released = true;
      }
    }

    if (s.down && (_now - s.pressedAt) >= BUTTON_REPEAT_MS &&
        (_now - s.lastRepeatAt) >= BUTTON_REPEAT_RATE) {
      s.repeated = true;
      s.lastRepeatAt = _now;
    }
  }
}

bool InputManager::anyDown() const {
  for (uint8_t i = 0; i < BTN_COUNT; ++i) if (_state[i].down) return true;
  return false;
}

bool InputManager::anyPressed() const {
  for (uint8_t i = 0; i < BTN_COUNT; ++i) if (_state[i].pressed) return true;
  return false;
}

bool InputManager::waitAllReleased(uint32_t timeoutMs) {
  uint32_t start = millis();
  while (millis() - start < timeoutMs) {
    update();
    if (!anyDown()) return true;
    delay(8);
  }
  return false;
}

int InputManager::navStep() const {
  const ButtonState& l1 = _state[BTN_L1];
  const ButtonState& l2 = _state[BTN_L2];
  if (l1.pressed || l1.repeated) return -1;
  if (l2.pressed || l2.repeated) return 1;
  return 0;
}

void InputManager::clearEvents() {
  for (uint8_t i = 0; i < BTN_COUNT; ++i) {
    _state[i].pressed = false;
    _state[i].released = false;
    _state[i].repeated = false;
  }
}
