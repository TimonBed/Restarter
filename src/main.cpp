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
bool Networking_clearConfig();
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

  // Factory reset button (active low with internal pull-up)
  pinMode(Config::PIN_FACTORY_RESET, INPUT_PULLUP);

  // Start network, UI, and MQTT subsystems.
  Networking_setup();
  WebInterface_setup();
  MqttHandler_setup();
  // set pin 3 to high
  digitalWrite(3, HIGH);

  // Placeholder: Signed OTA update validation hook.
}

// Factory reset button state tracking
static uint32_t s_resetButtonPressedMs = 0;
static bool s_resetButtonWasPressed = false;

void loop() {
  uint32_t loopStartUs = micros();

  // Check factory reset button (GPIO 9, active low)
  // LED on pin 10 provides visual feedback during hold
  bool resetButtonPressed = (digitalRead(Config::PIN_FACTORY_RESET) == LOW);
  static uint32_t s_lastResetLedToggleMs = 0;
  static bool s_resetLedState = false;
  
  if (resetButtonPressed) {
    if (!s_resetButtonWasPressed) {
      // Button just pressed - start timer
      s_resetButtonPressedMs = millis();
      s_resetButtonWasPressed = true;
      s_lastResetLedToggleMs = millis();
      s_resetLedState = true;
      digitalWrite(Config::PIN_WIFI_ERROR_LED, HIGH);
    } else {
      uint32_t holdDuration = millis() - s_resetButtonPressedMs;
      
      if (holdDuration >= Config::FACTORY_RESET_HOLD_MS) {
        // Button held long enough - perform factory reset
        digitalWrite(Config::PIN_WIFI_ERROR_LED, HIGH); // Solid on before reset
        Serial.println("Factory reset triggered by button!");
        Networking_clearConfig();
        delay(100);
        ESP.restart();
      } else {
        // Blink LED faster as hold duration increases
        // Start at 500ms interval, decrease to 50ms at the end
        uint32_t progress = (holdDuration * 100) / Config::FACTORY_RESET_HOLD_MS;
        uint32_t blinkInterval = 500 - (progress * 45 / 10); // 500ms -> 50ms
        if (blinkInterval < 50) blinkInterval = 50;
        
        if (millis() - s_lastResetLedToggleMs >= blinkInterval) {
          s_lastResetLedToggleMs = millis();
          s_resetLedState = !s_resetLedState;
          digitalWrite(Config::PIN_WIFI_ERROR_LED, s_resetLedState ? HIGH : LOW);
        }
      }
    }
  } else {
    if (s_resetButtonWasPressed) {
      // Button released before timeout - restore normal LED state
      s_resetButtonWasPressed = false;
      // LED control returns to Networking_loop
    }
  }
  
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
