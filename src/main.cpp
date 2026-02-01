/**
 * =============================================================================
 * main.cpp - Application Entry Point
 * =============================================================================
 * 
 * This is the main file that initializes and runs the PC Restarter firmware.
 * 
 * PROGRAM FLOW:
 * 
 *   1. setup() runs once at boot:
 *      - Initialize serial for debugging
 *      - Initialize hardware (relays, sensors, buttons)
 *      - Start WiFi (connects to router or creates AP for setup)
 *      - Start web server and MQTT
 * 
 *   2. loop() runs continuously:
 *      - Check factory reset button
 *      - Update PC state (read power LED, manage relays)
 *      - Handle network tasks (WiFi, web server, MQTT)
 *      - Broadcast status to connected clients
 * 
 * =============================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>

#include "Config.h"
#include "Constants.h"
#include "PCController.h"
#include "TempSensor.h"

// =============================================================================
// GLOBAL OBJECTS
// =============================================================================
// These are shared across all modules in the application.

StoredConfig g_config;              // Configuration (WiFi, MQTT, timing)
RuntimeState g_state;               // Current runtime status

AsyncWebServer g_server(80);        // HTTP server on port 80
AsyncWebSocket g_ws("/ws");         // WebSocket for real-time updates
WiFiClient g_wifiClient;            // TCP client for MQTT
PubSubClient g_mqttClient(g_wifiClient);  // MQTT client
PCController g_pc;                  // PC power/reset controller
TempSensor g_tempSensor;            // TMP112 temperature sensor

// Restart scheduling (used after saving config)
bool g_restartPending = false;      // True if restart is scheduled
uint32_t g_restartAtMs = 0;         // When to restart (millis timestamp)

// =============================================================================
// CPU LOAD TRACKING
// =============================================================================
// These variables help calculate approximate CPU usage percentage.

static uint32_t s_lastLoopUs = 0;       // Not used currently
static uint32_t s_loopTimeAccumUs = 0;  // Accumulated loop execution time
static uint32_t s_loopCount = 0;        // Number of loops counted
static uint32_t s_lastCpuCalcMs = 0;    // Last CPU calculation timestamp

// =============================================================================
// FACTORY RESET BUTTON STATE
// =============================================================================
// Tracks the physical reset button (GPIO 9) for factory reset.

static uint32_t s_resetButtonPressedMs = 0;  // When button was first pressed
static bool s_resetButtonWasPressed = false; // Was button pressed last loop?

// =============================================================================
// FORWARD DECLARATIONS
// =============================================================================
// Functions defined in other files that we call from here.

void Networking_setup();            // Initialize WiFi/AP mode
void Networking_loop();             // Handle WiFi connection
bool Networking_clearConfig();      // Erase all saved settings
void WebInterface_setup();          // Start web server
void WebInterface_loop();           // Handle web requests
void WebInterface_broadcastStatus(); // Push status to WebSocket clients
void MqttHandler_setup();           // Initialize MQTT
void MqttHandler_loop();            // Handle MQTT connection
void MqttHandler_publishState();    // Publish state to MQTT

// =============================================================================
// SETUP - Runs once at boot
// =============================================================================
void setup() {
  // --- Serial Monitor ---
  // Useful for debugging. Open with: pio device monitor --baud 115200
  Serial.begin(115200);
  Serial.println("\n\n=== PC Restarter Starting ===");
  
  // --- Hardware Initialization ---
  g_pc.begin();                     // Initialize relays and LED inputs
  g_tempSensor.begin(0, 1);         // TMP112 on GPIO 0 (SDA) and 1 (SCL)
  pinMode(Config::PIN_HDD_LED, INPUT_PULLUP);  // HDD LED input
  pinMode(Config::PIN_FACTORY_RESET, INPUT_PULLUP);  // Reset button (active low)
  
  // --- Network & Services ---
  Networking_setup();               // WiFi or AP mode
  WebInterface_setup();             // Web server and API endpoints
  MqttHandler_setup();              // MQTT for Home Assistant
  
  // --- Misc ---
  digitalWrite(3, HIGH);            // Placeholder for future use
  
  Serial.println("=== Setup Complete ===\n");
}

// =============================================================================
// LOOP - Runs continuously
// =============================================================================
void loop() {
  uint32_t loopStartUs = micros();  // Track loop execution time

  // -------------------------------------------------------------------------
  // FACTORY RESET BUTTON (GPIO 9)
  // -------------------------------------------------------------------------
  // Hold the button for 5 seconds to clear all settings and restart.
  // The LED (GPIO 10) blinks faster as you hold longer.
  
  bool resetButtonPressed = (digitalRead(Config::PIN_FACTORY_RESET) == LOW);
  static uint32_t s_lastResetLedToggleMs = 0;
  static bool s_resetLedState = false;
  
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
  
  // -------------------------------------------------------------------------
  // PC STATE MONITORING
  // -------------------------------------------------------------------------
  // Read inputs and update the PC controller state machine.
  
  g_pc.update();  // This reads power LED and manages relay timing
  
  // Copy state to global RuntimeState for web/MQTT
  g_state.pcState = g_pc.state();
  g_state.powerRelayActive = g_pc.powerRelayActive();
  g_state.resetRelayActive = g_pc.resetRelayActive();
  g_state.temperature = g_tempSensor.readTemperature();
  
  // Track HDD activity (when was the HDD LED last active?)
  bool hddActive = digitalRead(Config::PIN_HDD_LED) == 
                   (Config::HDD_LED_ACTIVE_HIGH ? HIGH : LOW);
  if (hddActive) {
    g_state.lastHddActiveMs = millis();
  }
  
  // -------------------------------------------------------------------------
  // ESP32 SYSTEM STATS
  // -------------------------------------------------------------------------
  // Update memory and CPU usage for the web interface.
  
  g_state.freeHeap = ESP.getFreeHeap();
  g_state.totalHeap = ESP.getHeapSize();
  
  // Calculate approximate CPU load (% of time spent in loop vs idle)
  uint32_t loopDurationUs = micros() - loopStartUs;
  s_loopTimeAccumUs += loopDurationUs;
  s_loopCount++;
  
  if (millis() - s_lastCpuCalcMs >= 1000) {
    // Calculate once per second
    uint32_t totalTimeUs = (millis() - s_lastCpuCalcMs) * 1000;
    if (totalTimeUs > 0) {
      g_state.cpuLoad = (uint8_t)((s_loopTimeAccumUs * 100) / totalTimeUs);
      if (g_state.cpuLoad > 100) g_state.cpuLoad = 100;
    }
    s_loopTimeAccumUs = 0;
    s_loopCount = 0;
    s_lastCpuCalcMs = millis();
  }

  // -------------------------------------------------------------------------
  // NETWORK SERVICES
  // -------------------------------------------------------------------------
  // Keep WiFi, web server, and MQTT running.
  
  Networking_loop();      // Handle WiFi connection/reconnection
  WebInterface_loop();    // Clean up stale WebSocket clients
  MqttHandler_loop();     // Handle MQTT connection/messages

  // -------------------------------------------------------------------------
  // STATUS BROADCAST
  // -------------------------------------------------------------------------
  // Push status updates to web clients and MQTT at regular intervals.
  
  static uint32_t lastStatusMs = 0;
  if (millis() - lastStatusMs > Config::STATUS_BROADCAST_MS) {
    lastStatusMs = millis();
    WebInterface_broadcastStatus();   // Send to WebSocket clients
    MqttHandler_publishState();       // Send to MQTT broker
    
    // Debug output to serial
    Serial.print("Temp: ");
    Serial.print(g_tempSensor.readTemperature());
    Serial.println(" C");
  }

  // -------------------------------------------------------------------------
  // SCHEDULED RESTART
  // -------------------------------------------------------------------------
  // After saving new config, we schedule a restart to apply changes.
  
  if (g_restartPending && millis() > g_restartAtMs) {
    Serial.println("Restarting to apply new configuration...");
    ESP.restart();
  }
}
