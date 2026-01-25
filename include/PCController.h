#pragma once

#include <Arduino.h>
#include "Config.h"
#include "Constants.h"

class PCController {
public:
  void begin();
  void update();

  PCState state() const;
  bool powerRelayActive() const;
  bool resetRelayActive() const;

  void pulsePower();
  void pulseReset();
  void setOutputsInactive();

private:
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
