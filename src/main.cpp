#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>

#include "Config.h"
#include "Constants.h"
#include "PCController.h"
#include "TempSensor.h"

// Global singletons used across modules.

StoredConfig g_config;
RuntimeState g_state;

AsyncWebServer g_server(80);     // HTTP server for UI and API
AsyncWebSocket g_ws("/ws");      // WebSocket for live status updates
WiFiClient g_wifiClient;         // TCP client used by MQTT
PubSubClient g_mqttClient(g_wifiClient); // MQTT client
PCController g_pc;               // PC control + state machine
TempSensor g_tempSensor;         // TMP112 temperature sensor

bool g_restartPending = false;
uint32_t g_restartAtMs = 0;

// CPU load tracking
static uint32_t s_lastLoopUs = 0;
static uint32_t s_loopTimeAccumUs = 0;
static uint32_t s_loopCount = 0;
static uint32_t s_lastCpuCalcMs = 0;

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
  g_tempSensor.begin(0, 1);  // SDA=GPIO0, SCL=GPIO1
  pinMode(Config::PIN_HDD_LED, INPUT_PULLUP);

  // Start network, UI, and MQTT subsystems.
  Networking_setup();
  WebInterface_setup();
  MqttHandler_setup();
  // set pin 3 to high
  digitalWrite(3, HIGH);

  // Placeholder: Signed OTA update validation hook.
}

void loop() {
  uint32_t loopStartUs = micros();
  
  // Read inputs and update state machine.
  g_pc.update();
  g_state.pcState = g_pc.state();
  g_state.powerRelayActive = g_pc.powerRelayActive();
  g_state.resetRelayActive = g_pc.resetRelayActive();
  g_state.temperature = g_tempSensor.readTemperature();
  
  // Track HDD LED last active time
  bool hddActive = digitalRead(Config::PIN_HDD_LED) == (Config::HDD_LED_ACTIVE_HIGH ? HIGH : LOW);
  if (hddActive) {
    g_state.lastHddActiveMs = millis();
  }
  
  // Update ESP32 memory stats
  g_state.freeHeap = ESP.getFreeHeap();
  g_state.totalHeap = ESP.getHeapSize();
  
  // Calculate CPU load (based on loop busy time vs total time)
  uint32_t loopDurationUs = micros() - loopStartUs;
  s_loopTimeAccumUs += loopDurationUs;
  s_loopCount++;
  
  if (millis() - s_lastCpuCalcMs >= 1000) {
    // Calculate CPU load: busy time / total time * 100
    uint32_t totalTimeUs = (millis() - s_lastCpuCalcMs) * 1000;
    if (totalTimeUs > 0) {
      g_state.cpuLoad = (uint8_t)((s_loopTimeAccumUs * 100) / totalTimeUs);
      if (g_state.cpuLoad > 100) g_state.cpuLoad = 100;
    }
    s_loopTimeAccumUs = 0;
    s_loopCount = 0;
    s_lastCpuCalcMs = millis();
  }

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
    Serial.print("Temp: ");
    Serial.print(g_tempSensor.readTemperature());
    Serial.println(" C");
  }

  // Restart after saving new config.
  if (g_restartPending && millis() > g_restartAtMs) {
    ESP.restart();
  }
}
