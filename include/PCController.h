#pragma once

#include <Arduino.h>
#include "Config.h"
#include "Constants.h"

class PCController {
public:
  // Initialize pins and safe defaults.
  void begin();
  // Call frequently to update state and handle pulses.
  void update();

  // Current derived PC state.
  PCState state() const;
  // Whether the power/reset relays are currently active.
  bool powerRelayActive() const;
  bool resetRelayActive() const;

  // Trigger a short press on the PC power/reset pins.
  void pulsePower();
  void pulseReset();
  // Force shutdown: hold power for 11 seconds.
  void forcePower();
  // Force relays to inactive (fail-safe).
  void setOutputsInactive();

private:
  // Low-level helpers.
  void setRelay(uint8_t pin, bool active, bool activeHigh);
  bool readActive(uint8_t pin, bool activeHigh) const;
  void updateState(uint32_t nowMs);

  uint32_t lastPowerOnMs = 0;
  uint32_t powerPulseUntilMs = 0;
  uint32_t resetPulseUntilMs = 0;
  bool lastPowerSignal = false;
  bool currentPowerSignal = false;
  PCState currentState = PCState::OFF;
};
