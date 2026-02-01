/**
 * =============================================================================
 * Constants.h - Data Structures & Type Definitions
 * =============================================================================
 * 
 * This file defines the data structures used throughout the application:
 *   - PCState: The current state of the PC (off, booting, running, etc.)
 *   - StoredConfig: Settings saved to flash (WiFi, MQTT, timing)
 *   - RuntimeState: Current status shared with the web UI and MQTT
 * 
 * These structures are used by multiple modules, so they're defined here
 * in a shared header file.
 * 
 * =============================================================================
 */

#pragma once

#include <Arduino.h>

// =============================================================================
// PC STATE ENUMERATION
// =============================================================================
/**
 * Represents the current state of the monitored PC.
 * The state is determined by reading the power LED and tracking timing.
 * 
 * State Machine:
 *   OFF ──[power LED on]──> BOOTING ──[grace period ends]──> RUNNING
 *    ↑                                                          │
 *    └──────────────────[power LED off]─────────────────────────┘
 *   
 *   Any state can transition to RESTARTING when a relay is activated.
 */
enum class PCState : uint8_t {
  OFF = 0,        // PC is powered off (power LED is off)
  BOOTING,        // PC just turned on, waiting for boot to complete
  RUNNING,        // PC is fully running (past boot grace period)
  RESTARTING,     // A power/reset action is in progress
};

// =============================================================================
// STORED CONFIGURATION
// =============================================================================
/**
 * Configuration that persists across reboots.
 * Stored in ESP32's NVS (Non-Volatile Storage), which survives:
 *   - Reboots
 *   - Firmware updates
 *   - Filesystem uploads
 * 
 * Only a factory reset clears this data.
 */
struct StoredConfig {
  // --- WiFi Settings ---
  String wifiSsid;      // Your WiFi network name
  String wifiPass;      // Your WiFi password
  
  // --- MQTT Settings (optional) ---
  String mqttHost;      // MQTT broker address (IP or hostname)
  uint16_t mqttPort = 1883;  // MQTT broker port (default: 1883)
  String mqttUser;      // MQTT username (optional)
  String mqttPass;      // MQTT password (optional)
  
  // --- Timing Settings (milliseconds) ---
  uint32_t powerPulseMs = 500;    // Power button press duration
  uint32_t resetPulseMs = 500;    // Reset button press duration
  uint32_t bootGraceMs = 60000;   // Time to wait before marking PC as "running"
};

// =============================================================================
// RUNTIME STATE
// =============================================================================
/**
 * Current device status, updated continuously and shared with:
 *   - Web UI (via WebSocket for real-time updates)
 *   - MQTT (for Home Assistant integration)
 *   - REST API (for polling)
 * 
 * This is NOT saved to flash - it's recalculated on every boot.
 */
struct RuntimeState {
  // --- Device Identity ---
  String hostname;      // Network hostname (e.g., "restarter-a1b2c3")
  String deviceId;      // Unique ID derived from MAC address
  
  // --- Network Status ---
  bool apMode = false;          // True if in Access Point mode (setup mode)
  bool wifiConnected = false;   // True if connected to WiFi router
  
  // --- PC Status ---
  PCState pcState = PCState::OFF;    // Current PC state
  bool powerRelayActive = false;     // Is power relay currently triggered?
  bool resetRelayActive = false;     // Is reset relay currently triggered?
  float temperature = 0.0f;          // Temperature from TMP112 sensor (°C)
  uint32_t lastHddActiveMs = 0;      // Last time HDD LED was active (millis)
  
  // --- ESP32 System Stats ---
  uint32_t freeHeap = 0;    // Available RAM (bytes)
  uint32_t totalHeap = 0;   // Total RAM (bytes)
  uint8_t cpuLoad = 0;      // Approximate CPU usage (0-100%)
};

// =============================================================================
// GLOBAL INSTANCES
// =============================================================================
// These are defined in main.cpp and accessed by other modules
extern StoredConfig g_config;  // Current configuration
extern RuntimeState g_state;   // Current runtime state
