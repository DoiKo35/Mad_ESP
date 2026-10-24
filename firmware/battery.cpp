#include "battery.h"
#include "config.h"

// Typical resting Li-Ion open-circuit voltage curve, using a conservative
// load-compensated approximation for a small ESP32 handheld.
static const float kCurveVoltage[] = {
  3.30f, 3.45f, 3.55f, 3.62f, 3.68f, 3.72f, 3.76f, 3.80f,
  3.84f, 3.88f, 3.92f, 3.96f, 4.00f, 4.05f, 4.10f, 4.16f, 4.20f
};
static const uint8_t kCurvePercent[] = {
  0, 2, 4, 8, 14, 22, 31, 40, 49, 58, 67, 75, 82, 88, 93, 97, 100
};

void BatteryMonitor::begin() {
  if (PIN_BATTERY_ADC >= 0) {
    pinMode(PIN_BATTERY_ADC, INPUT);
    analogSetPinAttenuation(PIN_BATTERY_ADC, ADC_11db);
  }
  if (PIN_CHARGE_DETECT >= 0) {
    pinMode(PIN_CHARGE_DETECT, INPUT_PULLUP);
    _chargeCandidate = CHARGE_ACTIVE_LOW
        ? (digitalRead(PIN_CHARGE_DETECT) == LOW)
        : (digitalRead(PIN_CHARGE_DETECT) == HIGH);
    _charging = _chargeCandidate;
  }
  _lastSampleAt = millis() - BATTERY_SAMPLE_MS;
  _lastChargeAt = millis();
  update();
}

void BatteryMonitor::readVoltage() {
  if (PIN_BATTERY_ADC < 0) {
    _valid = false;
    _percent = -1;
    return;
  }

  uint32_t sum = 0;
  uint8_t count = 0;
  for (uint8_t i = 0; i < BATTERY_ADC_SAMPLES; ++i) {
    uint32_t mv = analogReadMilliVolts(PIN_BATTERY_ADC);
    if (mv > 50) {
      sum += mv;
      ++count;
    }
    delayMicroseconds(120);
  }
  if (count == 0) {
    _valid = false;
    _percent = -1;
    return;
  }

  float pinVolts = (sum / (float)count) / 1000.0f;
  float batteryVolts = pinVolts * BATTERY_DIVIDER_K * BATTERY_CALIBRATION;

  if (batteryVolts < 2.4f || batteryVolts > 4.6f) {
    _valid = false;
    _percent = -1;
    return;
  }

  if (!_valid) _voltage = batteryVolts;
  else _voltage += (batteryVolts - _voltage) * BATTERY_FILTER;

  _valid = true;
  _millivolts = (uint32_t)(_voltage * 1000.0f + 0.5f);
  _percent = voltageToPercent(_voltage);
}

void BatteryMonitor::readChargePin() {
  if (PIN_CHARGE_DETECT < 0) {
    _charging = false;
    return;
  }
  bool low = CHARGE_ACTIVE_LOW ? (digitalRead(PIN_CHARGE_DETECT) == LOW)
                               : (digitalRead(PIN_CHARGE_DETECT) == HIGH);
  if (low == _chargeCandidate) {
    if (_chargeStable < 8) ++_chargeStable;
  } else {
    _chargeCandidate = low;
    _chargeStable = 0;
  }
  if (_chargeStable >= 3) _charging = _chargeCandidate;
}

void BatteryMonitor::update() {
  uint32_t now = millis();
  if (now - _lastSampleAt >= BATTERY_SAMPLE_MS) {
    _lastSampleAt = now;
    readVoltage();
  }
  if (PIN_CHARGE_DETECT >= 0 && now - _lastChargeAt >= 120) {
    _lastChargeAt = now;
    readChargePin();
  }
}

int BatteryMonitor::voltageToPercent(float volts) {
  const int count = sizeof(kCurveVoltage) / sizeof(kCurveVoltage[0]);
  if (volts <= kCurveVoltage[0]) return 0;
  if (volts >= kCurveVoltage[count - 1]) return 100;
  for (int i = 1; i < count; ++i) {
    if (volts <= kCurveVoltage[i]) {
      float span = kCurveVoltage[i] - kCurveVoltage[i - 1];
      float t = (volts - kCurveVoltage[i - 1]) / span;
      return (int)(kCurvePercent[i - 1] +
                   t * (float)(kCurvePercent[i] - kCurvePercent[i - 1]) + 0.5f);
    }
  }
  return 100;
}
