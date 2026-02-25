/**
 * =============================================================================
 * PCController.h - PC Power & Reset Control
 * =============================================================================
 * 
 * This class handles all interaction with the PC:
 *   - Reading the power LED to determine if PC is on/off
 *   - Triggering the power button (short press or force shutdown)
 *   - Triggering the reset button
 *   - Tracking the PC's state (off, booting, running)
 * 
 * HOW IT WORKS:
 * 
 * The ESP32 connects to the PC's front panel header, intercepting the
 * power and reset switch signals. Relays act as "virtual fingers" that
 * can press these buttons remotely.
 * 
 *   PC Front Panel          ESP32
 *   ┌─────────────┐        ┌──────┐
 *   │ Power LED ──┼──[Opto]─→ GPIO │  (read PC state)
 *   │ Power SW  ──┼──[Relay]← GPIO │  (press power button)
 *   │ Reset SW  ──┼──[Relay]← GPIO │  (press reset button)
 *   └─────────────┘        └──────┘
 * 
 * =============================================================================
 */

#pragma once

#include <Arduino.h>
#include "Config.h"
#include "Constants.h"

class PCController {
public:
  // =========================================================================
  // PUBLIC METHODS
  // =========================================================================
  
  /**
   * Initialize GPIO pins and set safe defaults.
   * Call this once in setup().
   */
  void begin();
  
  /**
   * Main update function - call this frequently in loop().
   * Reads inputs, manages relay timing, and updates PC state.
   */
  void update();

  /**
   * Get the current PC state (OFF, BOOTING, RUNNING, RESTARTING).
   */
  PCState state() const;
  
  /**
   * Check if the power relay is currently active (button being "held").
   */
  bool powerRelayActive() const;
  
  /**
   * Check if the reset relay is currently active.
   */
  bool resetRelayActive() const;

  /**
   * Trigger a short power button press.
   * Duration is set by config.powerPulseMs (default 500ms).
   * Use this to turn PC on/off normally.
   */
  void pulsePower();
  
  /**
   * Trigger a short reset button press.
   * Duration is set by config.resetPulseMs (default 500ms).
   */
  void pulseReset();
  
  /**
   * Force shutdown: hold power button for 11 seconds.
   * This forces the PC off even if it's frozen.
   * WARNING: This is like pulling the power cord - may cause data loss!
   */
  void forcePower();
  
  /**
   * Immediately deactivate both relays.
   * Used as a fail-safe and during initialization.
   */
  void setOutputsInactive();

private:
  // =========================================================================
  // PRIVATE HELPER METHODS
  // =========================================================================
  
  /**
   * Set a relay to active or inactive state.
   * Handles polarity conversion (active high vs active low).
   */
  void setRelay(uint8_t pin, bool active, bool activeHigh);
  
  /**
   * Read an input pin and return whether it's "active".
   * Handles polarity conversion for optocoupler signals.
   */
  bool readActive(uint8_t pin, bool activeHigh) const;
  
  /**
   * Update the internal state machine based on inputs and timing.
   */
  void updateState(uint32_t nowMs);

  // =========================================================================
  // PRIVATE STATE VARIABLES
  // =========================================================================
  
  uint32_t lastPowerOnMs = 0;       // When the power LED last turned ON
  uint32_t powerPulseUntilMs = 0;   // When to release the power relay (0 = inactive)
  uint32_t resetPulseUntilMs = 0;   // When to release the reset relay (0 = inactive)
  bool powerRelayLatched = false;
  bool resetRelayLatched = false;
  bool lastPowerSignal = false;     // Previous power LED state (for edge detection)
  bool currentPowerSignal = false;  // Current power LED state
  PCState currentState = PCState::OFF;  // Current derived PC state
};
