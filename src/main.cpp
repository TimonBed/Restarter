/**
 * =============================================================================
 * main.cpp - Application Entry Point
 * =============================================================================
 * 
 * Initializes and runs the PC Restarter firmware.
 * 
 * setup() - Hardware init, WiFi, web server, MQTT
 * loop()  - Factory reset, PC state, network services, status broadcast
 * 
 * =============================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>

#include "Config.h"
#include "Constants.h"
#include "PCController.h"
#include "TempSensor.h"
#include "FactoryReset.h"

// =============================================================================
// GLOBAL OBJECTS
// =============================================================================

StoredConfig g_config;
RuntimeState g_state;

AsyncWebServer g_server(80);
AsyncWebSocket g_ws("/ws");
WiFiClient g_wifiClient;           // Standard client for non-TLS MQTT
WiFiClientSecure g_wifiClientTls;  // Secure client for TLS MQTT
PubSubClient g_mqttClient;         // Will be configured with appropriate client
PCController g_pc;
TempSensor g_tempSensor;

bool g_restartPending = false;
uint32_t g_restartAtMs = 0;

// =============================================================================
// EXTERNAL FUNCTIONS
// =============================================================================

void Networking_setup();
void Networking_loop();
void WebInterface_setup();
void WebInterface_loop();
void WebInterface_broadcastStatus();
void MqttHandler_setup();
void MqttHandler_loop();
void MqttHandler_publishState();

// =============================================================================
// LOCAL HELPERS
// =============================================================================

static uint32_t s_loopTimeAccumUs = 0;
static uint32_t s_loopCount = 0;
static uint32_t s_lastCpuCalcMs = 0;

/**
 * Update PC controller and sync state to global RuntimeState
 */
static void updatePCState() {
  g_pc.update();
  
  g_state.pcState = g_pc.state();
  g_state.powerRelayActive = g_pc.powerRelayActive();
  g_state.resetRelayActive = g_pc.resetRelayActive();
  g_state.temperature = g_tempSensor.readTemperature();
  
  bool hddActive = digitalRead(Config::PIN_HDD_LED) == 
                   (Config::HDD_LED_ACTIVE_HIGH ? HIGH : LOW);
  if (hddActive) {
    g_state.lastHddActiveMs = millis();
  }
}

/**
 * Update ESP32 system stats (memory, CPU load)
 */
static void updateSystemStats(uint32_t loopDurationUs) {
  g_state.freeHeap = ESP.getFreeHeap();
  g_state.totalHeap = ESP.getHeapSize();
  
  s_loopTimeAccumUs += loopDurationUs;
  s_loopCount++;
  
  if (millis() - s_lastCpuCalcMs >= 1000) {
    uint32_t totalTimeUs = (millis() - s_lastCpuCalcMs) * 1000;
    if (totalTimeUs > 0) {
      g_state.cpuLoad = (uint8_t)((s_loopTimeAccumUs * 100) / totalTimeUs);
      if (g_state.cpuLoad > 100) g_state.cpuLoad = 100;
    }
    s_loopTimeAccumUs = 0;
    s_loopCount = 0;
    s_lastCpuCalcMs = millis();
  }
}

/**
 * Broadcast status to WebSocket and MQTT at configured interval
 */
static void broadcastStatus() {
  static uint32_t lastMs = 0;
  if (millis() - lastMs > Config::STATUS_BROADCAST_MS) {
    lastMs = millis();
    WebInterface_broadcastStatus();
    MqttHandler_publishState();
  }
}

/**
 * Handle scheduled restart (after config save)
 */
static void handleScheduledRestart() {
  if (g_restartPending && millis() > g_restartAtMs) {
    Serial.println("Restarting to apply new configuration...");
    ESP.restart();
  }
}

// =============================================================================
// SETUP
// =============================================================================

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=== PC Restarter Starting ===");
  
  // Hardware
  g_pc.begin();
  g_tempSensor.begin(0, 1);
  pinMode(Config::PIN_HDD_LED, INPUT_PULLUP);
  FactoryReset_setup();
  
  // Network & Services
  Networking_setup();
  WebInterface_setup();
  MqttHandler_setup();
  
  Serial.println("=== Setup Complete ===\n");
}

// =============================================================================
// LOOP
// =============================================================================

void loop() {
  uint32_t loopStartUs = micros();

  FactoryReset_loop();
  updatePCState();
  
  Networking_loop();
  WebInterface_loop();
  MqttHandler_loop();
  
  broadcastStatus();
  handleScheduledRestart();
  
  updateSystemStats(micros() - loopStartUs);
  
  delay(5);  // Yield to RTOS, prevents 100% CPU
}
