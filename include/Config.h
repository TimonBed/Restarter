/**
 * =============================================================================
 * Config.h - Hardware Configuration & Default Settings
 * =============================================================================
 * 
 * This file contains all the hardware pin assignments and default timing values.
 * 
 * IMPORTANT FOR BEGINNERS:
 * - Modify the GPIO pins below to match YOUR wiring
 * - All timing values are in milliseconds (1000ms = 1 second)
 * - Signal polarity depends on your relay/optocoupler type
 * 
 * =============================================================================
 */

#pragma once

#include <Arduino.h>

namespace Config {

// =============================================================================
// FIRMWARE VERSION
// =============================================================================
// Update this when you make changes to track which version is running
constexpr char FW_VERSION[] = "0.2.0";

// =============================================================================
// GPIO PIN ASSIGNMENTS
// =============================================================================
// Change these to match your wiring. The ESP32-C3 has GPIOs 0-10, 18-21.
//
// INPUTS (from PC):
//   - Power LED: Tells us if the PC is on/off
//   - HDD LED: Tells us if the PC is actively reading/writing
//   - Factory Reset Button: Hold to clear all settings
//
// OUTPUTS (to PC):
//   - Power Relay: Simulates pressing the power button
//   - Reset Relay: Simulates pressing the reset button
//   - Status LED: Shows WiFi/reset status

// --- Input Pins ---
constexpr uint8_t PIN_PWR_LED = 4;        // PC power LED (via optocoupler)
constexpr uint8_t PIN_HDD_LED = 5;        // PC HDD activity LED (via optocoupler)
constexpr uint8_t PIN_FACTORY_RESET = 9;  // Factory reset button (to GND)

// --- Output Pins ---
constexpr uint8_t PIN_RELAY_POWER = 6;    // Power button relay
constexpr uint8_t PIN_RELAY_RESET = 7;    // Reset button relay
constexpr uint8_t PIN_WIFI_ERROR_LED = 10; // Status LED (WiFi/reset feedback)

// =============================================================================
// SIGNAL POLARITY
// =============================================================================
// Set to 'true' if the signal is active when HIGH, 'false' if active when LOW.
//
// For OPTOCOUPLERS: Usually active LOW (the optocoupler pulls the GPIO low
//                   when the PC's LED turns on)
//
// For RELAYS: Check your relay module - most are active HIGH, but some
//             (especially those marked "active low") need 'false' here

constexpr bool PWR_LED_ACTIVE_HIGH = false;    // PC power LED signal
constexpr bool HDD_LED_ACTIVE_HIGH = false;    // PC HDD LED signal
constexpr bool POWER_RELAY_ACTIVE_HIGH = true; // Power relay trigger
constexpr bool RESET_RELAY_ACTIVE_HIGH = true; // Reset relay trigger

// =============================================================================
// TIMING CONFIGURATION (in milliseconds)
// =============================================================================
// These are default values - some can be changed via the web interface.

// --- Button Press Durations ---
constexpr uint32_t POWER_PULSE_MS = 500;          // How long to "press" power button
constexpr uint32_t RESET_PULSE_MS = 500;          // How long to "press" reset button
constexpr uint32_t FORCE_SHUTDOWN_PULSE_MS = 11000; // Force shutdown hold (11 seconds)

// --- Boot Detection ---
constexpr uint32_t BOOT_GRACE_MS = 60000;  // Time to wait after power-on before
                                           // considering PC "running" (60 seconds)

// --- Factory Reset ---
constexpr uint32_t FACTORY_RESET_HOLD_MS = 5000;  // Hold button this long to reset (5s)

// --- Network Timeouts ---
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000; // WiFi connection attempt timeout
constexpr uint32_t AP_IDLE_TIMEOUT_MS = 300000;     // Restart after 5 min idle in AP mode
constexpr uint32_t MQTT_RECONNECT_MS = 5000;        // Time between MQTT reconnect attempts

// --- Status Updates ---
constexpr uint32_t STATUS_BROADCAST_MS = 1000;  // How often to push status (1 second)

// =============================================================================
// STORAGE & IDENTITY
// =============================================================================
// These define how the device identifies itself on the network

constexpr char CONFIG_PATH[] = "/config.json";   // Legacy - now using NVS
constexpr char HOSTNAME_PREFIX[] = "restarter-"; // Network hostname prefix
constexpr char AP_SSID_PREFIX[] = "Restarter-";  // WiFi AP name prefix

} // namespace Config
