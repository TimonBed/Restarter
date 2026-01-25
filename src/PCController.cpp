#include <Arduino.h>
#include "PCController.h"

void PCController::begin() {
  pinMode(Config::PIN_PWR_LED, INPUT);
  pinMode(Config::PIN_RELAY_POWER, OUTPUT);
  pinMode(Config::PIN_RELAY_RESET, OUTPUT);
  setOutputsInactive();

  lastPowerOnMs = 0;
  lastPowerSignal = false;
  currentState = PCState::OFF;
}

void PCController::setRelay(uint8_t pin, bool active, bool activeHigh) {
  bool level = activeHigh ? active : !active;
  digitalWrite(pin, level ? HIGH : LOW);
}

bool PCController::readActive(uint8_t pin, bool activeHigh) const {
  bool level = digitalRead(pin) == HIGH;
  return activeHigh ? level : !level;
}

void PCController::setOutputsInactive() {
  setRelay(Config::PIN_RELAY_POWER, false, Config::POWER_RELAY_ACTIVE_HIGH);
  setRelay(Config::PIN_RELAY_RESET, false, Config::RESET_RELAY_ACTIVE_HIGH);
  powerPulseUntilMs = 0;
  resetPulseUntilMs = 0;
}

void PCController::pulsePower() {
  powerPulseUntilMs = millis() + Config::POWER_PULSE_MS;
  setRelay(Config::PIN_RELAY_POWER, true, Config::POWER_RELAY_ACTIVE_HIGH);
}

void PCController::pulseReset() {
  resetPulseUntilMs = millis() + Config::RESET_PULSE_MS;
  setRelay(Config::PIN_RELAY_RESET, true, Config::RESET_RELAY_ACTIVE_HIGH);
}

void PCController::update() {
  uint32_t nowMs = millis();

  currentPowerSignal = readActive(Config::PIN_PWR_LED, Config::PWR_LED_ACTIVE_HIGH);

  if (currentPowerSignal && !lastPowerSignal) {
    lastPowerOnMs = nowMs;
  }
  lastPowerSignal = currentPowerSignal;

  if (powerPulseUntilMs > 0 && nowMs >= powerPulseUntilMs) {
    setRelay(Config::PIN_RELAY_POWER, false, Config::POWER_RELAY_ACTIVE_HIGH);
    powerPulseUntilMs = 0;
  }
  if (resetPulseUntilMs > 0 && nowMs >= resetPulseUntilMs) {
    setRelay(Config::PIN_RELAY_RESET, false, Config::RESET_RELAY_ACTIVE_HIGH);
    resetPulseUntilMs = 0;
  }

  updateState(nowMs);
}

void PCController::updateState(uint32_t nowMs) {
  bool actionActive = (powerPulseUntilMs > 0) || (resetPulseUntilMs > 0);

  if (actionActive) {
    currentState = PCState::RESTARTING;
  } else if (!currentPowerSignal) {
    currentState = PCState::OFF;
  } else if ((nowMs - lastPowerOnMs) < Config::BOOT_GRACE_MS) {
    currentState = PCState::BOOTING;
  } else {
    currentState = PCState::RUNNING;
  }
}

PCState PCController::state() const {
  return currentState;
}

bool PCController::powerRelayActive() const {
  return powerPulseUntilMs > 0;
}

bool PCController::resetRelayActive() const {
  return resetPulseUntilMs > 0;
}
