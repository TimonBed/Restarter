#include <Arduino.h>
#include "PCController.h"
#include "Constants.h"

extern StoredConfig g_config;

void PCController::begin() {
  // Configure input/output pins and default safe output states.
  pinMode(Config::PIN_PWR_LED, INPUT);
  pinMode(Config::PIN_RELAY_POWER, OUTPUT);
  pinMode(Config::PIN_RELAY_RESET, OUTPUT);
  setOutputsInactive();

  lastPowerOnMs = 0;
  lastPowerSignal = false;
  currentState = PCState::OFF;
}

void PCController::setRelay(uint8_t pin, bool active, bool activeHigh) {
  // Convert "active" to actual output level.
  bool level = activeHigh ? active : !active;
  digitalWrite(pin, level ? HIGH : LOW);
}

bool PCController::readActive(uint8_t pin, bool activeHigh) const {
  bool level = digitalRead(pin) == HIGH;
  return activeHigh ? level : !level;
}

void PCController::setOutputsInactive() {
  // Fail-safe: outputs inactive on boot and after pulses.
  setRelay(Config::PIN_RELAY_POWER, false, Config::POWER_RELAY_ACTIVE_HIGH);
  setRelay(Config::PIN_RELAY_RESET, false, Config::RESET_RELAY_ACTIVE_HIGH);
  powerPulseUntilMs = 0;
  resetPulseUntilMs = 0;
}

void PCController::pulsePower() {
  // Short press for power switch (duration from config).
  powerPulseUntilMs = millis() + g_config.powerPulseMs;
  setRelay(Config::PIN_RELAY_POWER, true, Config::POWER_RELAY_ACTIVE_HIGH);
}

void PCController::pulseReset() {
  // Short press for reset switch (duration from config).
  resetPulseUntilMs = millis() + g_config.resetPulseMs;
  setRelay(Config::PIN_RELAY_RESET, true, Config::RESET_RELAY_ACTIVE_HIGH);
}

void PCController::update() {
  // Main periodic handler: reads inputs, manages relay pulses, and tracks PC state.
  uint32_t nowMs = millis();  // Get the current time in milliseconds since boot

  // Read the power LED state to determine whether PC is on
  currentPowerSignal = readActive(Config::PIN_PWR_LED, Config::PWR_LED_ACTIVE_HIGH);

  // Detect a rising edge on the power signal.
  // If the PC power LED just turned on, mark the time as the last boot event.
  if (currentPowerSignal && !lastPowerSignal) {
    // Power LED went from OFF to ON: treat as system boot.
    lastPowerOnMs = nowMs;
  }
  // Store last power signal so we can detect edges on next update.
  lastPowerSignal = currentPowerSignal;

  // Check if the power relay pulse (power button "press") should end.
  // If the configured pulse duration has expired, deactivate the relay.
  if (powerPulseUntilMs > 0 && nowMs >= powerPulseUntilMs) {
    setRelay(Config::PIN_RELAY_POWER, false, Config::POWER_RELAY_ACTIVE_HIGH);
    powerPulseUntilMs = 0;
  }
  // Similarly, check if the reset relay pulse should end.
  if (resetPulseUntilMs > 0 && nowMs >= resetPulseUntilMs) {
    setRelay(Config::PIN_RELAY_RESET, false, Config::RESET_RELAY_ACTIVE_HIGH);
    resetPulseUntilMs = 0;
  }

  // Update internal state machine based on input readings and time.
  updateState(nowMs);
}

void PCController::updateState(uint32_t nowMs) {
  // Simple state machine based on power LED and recent actions.
  bool actionActive = (powerPulseUntilMs > 0) || (resetPulseUntilMs > 0);

  if (actionActive) {
    currentState = PCState::RESTARTING;
  } else if (!currentPowerSignal) {
    currentState = PCState::OFF;
  } else if ((nowMs - lastPowerOnMs) < g_config.bootGraceMs) {
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
