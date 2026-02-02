/**
 * FactoryReset.h - Factory Reset Button Handler
 * 
 * Monitors the factory reset button (GPIO 9) and triggers a full
 * config wipe after holding for 5 seconds.
 */

#pragma once

#include <Arduino.h>

/**
 * Initialize factory reset button pin.
 * Call once in setup().
 */
void FactoryReset_setup();

/**
 * Check factory reset button state.
 * Call every loop iteration. Handles LED blinking feedback
 * and triggers reset after hold duration.
 */
void FactoryReset_loop();
