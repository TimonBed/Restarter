#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>

#include "Config.h"
#include "Constants.h"
#include "PCController.h"

// Global singletons used across modules.

StoredConfig g_config;
RuntimeState g_state;

AsyncWebServer g_server(80);     // HTTP server for UI and API
AsyncWebSocket g_ws("/ws");      // WebSocket for live status updates
WiFiClient g_wifiClient;         // TCP client used by MQTT
PubSubClient g_mqttClient(g_wifiClient); // MQTT client
PCController g_pc;               // PC control + state machine

bool g_restartPending = false;
uint32_t g_restartAtMs = 0;

void Networking_setup();
void Networking_loop();
void WebInterface_setup();
void WebInterface_loop();
void WebInterface_broadcastStatus();
void MqttHandler_setup();
void MqttHandler_loop();
void MqttHandler_publishState();

void setup() {
  // Serial is helpful for debugging on first bring-up.
  Serial.begin(115200);
  g_pc.begin();

  // Start network, UI, and MQTT subsystems.
  Networking_setup();
  WebInterface_setup();
  MqttHandler_setup();

  // Placeholder: Signed OTA update validation hook.
}

void loop() {
  // Read inputs and update state machine.
  g_pc.update();
  g_state.pcState = g_pc.state();
  g_state.powerRelayActive = g_pc.powerRelayActive();
  g_state.resetRelayActive = g_pc.resetRelayActive();

  // Keep services alive.
  Networking_loop();
  WebInterface_loop();
  MqttHandler_loop();

  // Push status to UI/MQTT on a fixed interval.
  static uint32_t lastStatusMs = 0;
  if (millis() - lastStatusMs > Config::STATUS_BROADCAST_MS) {
    lastStatusMs = millis();
    WebInterface_broadcastStatus();
    MqttHandler_publishState();
  }

  // Restart after saving new config.
  if (g_restartPending && millis() > g_restartAtMs) {
    ESP.restart();
  }
}
