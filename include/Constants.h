#pragma once

#include <Arduino.h>

// High-level PC state derived from LEDs and actions.
enum class PCState : uint8_t {
  OFF = 0,
  BOOTING,
  RUNNING,
  RESTARTING,
};

// Stored configuration loaded/saved to NVS.
struct StoredConfig {
  String wifiSsid;
  String wifiPass;
  String mqttHost;
  uint16_t mqttPort = 1883;
  String mqttUser;
  String mqttPass;
  // Timing settings (ms)
  uint32_t powerPulseMs = 500;
  uint32_t resetPulseMs = 250;
  uint32_t bootGraceMs = 60000;
};

// Current runtime status shared with UI/MQTT.
struct RuntimeState {
  String hostname;
  String deviceId;
  bool apMode = false;
  bool wifiConnected = false;
  PCState pcState = PCState::OFF;
  bool powerRelayActive = false;
  bool resetRelayActive = false;
};

extern StoredConfig g_config;
extern RuntimeState g_state;
