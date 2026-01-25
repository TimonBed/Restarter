#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>

#include "Config.h"
#include "Constants.h"
#include "PCController.h"

StoredConfig g_config;
RuntimeState g_state;

AsyncWebServer g_server(80);
AsyncWebSocket g_ws("/ws");
WiFiClient g_wifiClient;
PubSubClient g_mqttClient(g_wifiClient);
PCController g_pc;

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
  Serial.begin(115200);
  g_pc.begin();

  Networking_setup();
  WebInterface_setup();
  MqttHandler_setup();

  // Placeholder: Signed OTA update validation hook.
}

void loop() {
  g_pc.update();
  g_state.pcState = g_pc.state();
  g_state.powerRelayActive = g_pc.powerRelayActive();
  g_state.resetRelayActive = g_pc.resetRelayActive();

  Networking_loop();
  WebInterface_loop();
  MqttHandler_loop();

  static uint32_t lastStatusMs = 0;
  if (millis() - lastStatusMs > Config::STATUS_BROADCAST_MS) {
    lastStatusMs = millis();
    WebInterface_broadcastStatus();
    MqttHandler_publishState();
  }

  if (g_restartPending && millis() > g_restartAtMs) {
    ESP.restart();
  }
}
