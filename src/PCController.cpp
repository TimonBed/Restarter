/**
 * =============================================================================
 * PCController.cpp - PC Power & Reset Control Implementation
 * =============================================================================
 * 
 * This file implements the PC control logic:
 *   - Reading the PC's power LED to know if it's on or off
 *   - Controlling relays to simulate button presses
 *   - Tracking the PC's state (OFF → BOOTING → RUNNING)
 * 
 * HOW THE STATE MACHINE WORKS:
 * 
 *   1. When power LED turns ON, we transition to BOOTING
 *   2. After bootGraceMs (default 60s), we transition to RUNNING
 *   3. When power LED turns OFF, we transition to OFF
 *   4. When a relay is active, we show RESTARTING
 * 
 * =============================================================================
 */

#include <Arduino.h>
#include "PCController.h"
#include "Constants.h"

// Access the global configuration for timing values
extern StoredConfig g_config;

// =============================================================================
// INITIALIZATION
// =============================================================================

void PCController::begin() {
  // Configure GPIO pins for inputs and outputs
  
  // Power LED input (from PC via optocoupler)
  pinMode(Config::PIN_PWR_LED, INPUT_PULLUP);
  
  // Relay outputs (active high by default)
  pinMode(Config::PIN_RELAY_POWER, OUTPUT);
  pinMode(Config::PIN_RELAY_RESET, OUTPUT);
  
  // Ensure relays are OFF at startup (fail-safe)
  setOutputsInactive();

  // Initialize state tracking
  lastPowerOnMs = 0;
  lastPowerSignal = false;
  currentState = PCState::OFF;
}

// =============================================================================
// RELAY CONTROL
// =============================================================================

void PCController::setRelay(uint8_t pin, bool active, bool activeHigh) {
  /**
   * Set a relay to active or inactive state.
   * 
   * This handles the polarity conversion:
   *   - If relay is "active high": HIGH = relay ON, LOW = relay OFF
   *   - If relay is "active low":  LOW = relay ON, HIGH = relay OFF
   * 
   * @param pin       GPIO pin number
   * @param active    True to activate relay, false to deactivate
   * @param activeHigh True if relay triggers on HIGH signal
   */
  bool level = activeHigh ? active : !active;
  digitalWrite(pin, level ? HIGH : LOW);
}

bool PCController::readActive(uint8_t pin, bool activeHigh) const {
  /**
   * Read an input and determine if it's "active".
   * 
   * This handles polarity conversion for optocouplers:
   *   - Active HIGH: signal is active when GPIO reads HIGH
   *   - Active LOW:  signal is active when GPIO reads LOW
   *     (Most optocouplers pull the GPIO LOW when the LED is on)
   */
  bool level = digitalRead(pin) == HIGH;
  return activeHigh ? level : !level;
}

void PCController::setOutputsInactive() {
  /**
   * Turn off both relays immediately.
   * 
   * This is a fail-safe function called:
   *   - At startup (to ensure safe initial state)
   *   - After pulse durations expire
   *   - In case of any error condition
   */
  setRelay(Config::PIN_RELAY_POWER, false, Config::POWER_RELAY_ACTIVE_HIGH);
  setRelay(Config::PIN_RELAY_RESET, false, Config::RESET_RELAY_ACTIVE_HIGH);
  powerPulseUntilMs = 0;
  resetPulseUntilMs = 0;
}

// =============================================================================
// BUTTON ACTIONS
// =============================================================================

void PCController::pulsePower() {
  /**
   * Simulate a short power button press.
   * 
   * The relay activates immediately and stays on for powerPulseMs
   * (default 500ms = half a second), then automatically releases.
   * 
   * This is equivalent to pressing and releasing the power button.
   */
  powerPulseUntilMs = millis() + g_config.powerPulseMs;
  setRelay(Config::PIN_RELAY_POWER, true, Config::POWER_RELAY_ACTIVE_HIGH);
}

void PCController::pulseReset() {
  /**
   * Simulate a short reset button press.
   * 
   * Same as pulsePower() but for the reset button.
   * Duration is controlled by resetPulseMs (default 500ms).
   */
  resetPulseUntilMs = millis() + g_config.resetPulseMs;
  setRelay(Config::PIN_RELAY_RESET, true, Config::RESET_RELAY_ACTIVE_HIGH);
}

void PCController::forcePower() {
  /**
   * Force shutdown by holding power button for 11 seconds.
   * 
   * Most PCs will force power off if the power button is held for
   * 10+ seconds. This is useful when the PC is frozen and won't
   * respond to a normal power press.
   * 
   * WARNING: This is like pulling the power cord - it may cause
   * data loss or filesystem corruption!
   */
  powerPulseUntilMs = millis() + Config::FORCE_SHUTDOWN_PULSE_MS;
  setRelay(Config::PIN_RELAY_POWER, true, Config::POWER_RELAY_ACTIVE_HIGH);
}

// =============================================================================
// MAIN UPDATE LOOP
// =============================================================================

void PCController::update() {
  /**
   * Main periodic handler - call this frequently in loop().
   * 
   * This function:
   *   1. Reads the power LED input
   *   2. Detects power-on events (LED turning on)
   *   3. Manages relay pulse timing (auto-release after duration)
   *   4. Updates the PC state machine
   */
  uint32_t nowMs = millis();

  // -------------------------------------------------------------------------
  // Step 1: Read power LED state
  // -------------------------------------------------------------------------
  currentPowerSignal = readActive(Config::PIN_PWR_LED, Config::PWR_LED_ACTIVE_HIGH);

  // -------------------------------------------------------------------------
  // Step 2: Detect rising edge (power turning ON)
  // -------------------------------------------------------------------------
  // If the power LED just turned on (was off, now on), record the time.
  // This is used to track the boot grace period.
  if (currentPowerSignal && !lastPowerSignal) {
    lastPowerOnMs = nowMs;
    Serial.println("PC Power LED: ON (boot detected)");
  }
  
  // Save current state for next loop's edge detection
  lastPowerSignal = currentPowerSignal;

  // -------------------------------------------------------------------------
  // Step 3: Manage relay pulse timing
  // -------------------------------------------------------------------------
  // Check if power relay pulse should end
  if (powerPulseUntilMs > 0 && nowMs >= powerPulseUntilMs) {
    setRelay(Config::PIN_RELAY_POWER, false, Config::POWER_RELAY_ACTIVE_HIGH);
    powerPulseUntilMs = 0;
    Serial.println("Power relay: released");
  }
  
  // Check if reset relay pulse should end
  if (resetPulseUntilMs > 0 && nowMs >= resetPulseUntilMs) {
    setRelay(Config::PIN_RELAY_RESET, false, Config::RESET_RELAY_ACTIVE_HIGH);
    resetPulseUntilMs = 0;
    Serial.println("Reset relay: released");
  }

  // -------------------------------------------------------------------------
  // Step 4: Update state machine
  // -------------------------------------------------------------------------
  updateState(nowMs);
}

// =============================================================================
// STATE MACHINE
// =============================================================================

void PCController::updateState(uint32_t nowMs) {
  /**
   * Update the PC state based on inputs and timing.
   * 
   * States:
   *   - OFF:        Power LED is off
   *   - BOOTING:    Power LED on, but within boot grace period
   *   - RUNNING:    Power LED on, past boot grace period
   *   - RESTARTING: A relay is currently active (action in progress)
   */
  
  // Check if any action is in progress
  bool actionActive = (powerPulseUntilMs > 0) || (resetPulseUntilMs > 0);

  if (actionActive) {
    // While pressing a button, show RESTARTING
    currentState = PCState::RESTARTING;
  } else if (!currentPowerSignal) {
    // Power LED is off = PC is off
    currentState = PCState::OFF;
  } else if ((nowMs - lastPowerOnMs) < g_config.bootGraceMs) {
    // Power LED is on, but we're still within boot grace period
    currentState = PCState::BOOTING;
  } else {
    // Power LED is on and boot grace period has passed
    currentState = PCState::RUNNING;
  }
}

// =============================================================================
// STATE GETTERS
// =============================================================================

PCState PCController::state() const {
  return currentState;
}

bool PCController::powerRelayActive() const {
  return powerPulseUntilMs > 0;
}

bool PCController::resetRelayActive() const {
  return resetPulseUntilMs > 0;
}
