/**
 * =============================================================================
 * FactoryReset.cpp - Factory Reset Button Handler
 * =============================================================================
 * 
 * Monitors the factory reset button and triggers a full config wipe
 * when held for 5 seconds. LED blinks faster as hold duration increases.
 * 
 * =============================================================================
 */

#include "FactoryReset.h"
#include "Config.h"

// External function from Networking.cpp
extern bool Networking_clearConfig();

// =============================================================================
// BUTTON STATE TRACKING
// =============================================================================

static uint32_t s_resetButtonPressedMs = 0;  // When button was first pressed
static bool s_resetButtonWasPressed = false; // Was button pressed last loop?
static uint32_t s_lastResetLedToggleMs = 0;  // Last LED toggle timestamp
static bool s_resetLedState = false;         // Current LED state

// =============================================================================
// SETUP
// =============================================================================
void FactoryReset_setup() {
  pinMode(Config::PIN_FACTORY_RESET, INPUT_PULLUP);  // Reset button (active low)
}

// =============================================================================
// LOOP - Check button state each iteration
// =============================================================================
void FactoryReset_loop() {
  bool resetButtonPressed = (digitalRead(Config::PIN_FACTORY_RESET) == LOW);
  
  if (resetButtonPressed) {
    if (!s_resetButtonWasPressed) {
      // Button just pressed - start the timer
      s_resetButtonPressedMs = millis();
      s_resetButtonWasPressed = true;
      s_lastResetLedToggleMs = millis();
      s_resetLedState = true;
      digitalWrite(Config::PIN_WIFI_ERROR_LED, HIGH);
    } else {
      // Button is being held - check duration
      uint32_t holdDuration = millis() - s_resetButtonPressedMs;
      
      if (holdDuration >= Config::FACTORY_RESET_HOLD_MS) {
        // Button held long enough - perform factory reset!
        digitalWrite(Config::PIN_WIFI_ERROR_LED, HIGH);  // Solid LED
        Serial.println("!!! FACTORY RESET TRIGGERED !!!");
        Networking_clearConfig();  // Erase WiFi, MQTT, and timing settings
        delay(100);
        ESP.restart();             // Reboot into AP mode
      } else {
        // Still holding - blink LED faster as progress increases
        // Blink interval: 500ms at start → 50ms at end
        uint32_t progress = (holdDuration * 100) / Config::FACTORY_RESET_HOLD_MS;
        uint32_t blinkInterval = 500 - (progress * 45 / 10);
        if (blinkInterval < 50) blinkInterval = 50;
        
        if (millis() - s_lastResetLedToggleMs >= blinkInterval) {
          s_lastResetLedToggleMs = millis();
          s_resetLedState = !s_resetLedState;
          digitalWrite(Config::PIN_WIFI_ERROR_LED, s_resetLedState ? HIGH : LOW);
        }
      }
    }
  } else {
    // Button released (or not pressed)
    if (s_resetButtonWasPressed) {
      s_resetButtonWasPressed = false;
      // LED control returns to Networking_loop()
    }
  }
}
