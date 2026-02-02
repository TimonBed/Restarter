#pragma once
#include <Arduino.h>

// PC state
enum class PCState : uint8_t {
  OFF = 0,
  BOOTING,
  RUNNING,
  RESTARTING,
};

// Settings stored in NVS/flash
struct StoredConfig {
  String wifiSsid;
  String wifiPass;
  String mqttHost;
  uint16_t mqttPort = 1883;
  String mqttUser;
  String mqttPass;
  uint32_t powerPulseMs = 500;
  uint32_t resetPulseMs = 500;
  uint32_t bootGraceMs = 60000;
};

// Current runtime status (not persistent)
struct RuntimeState {
  String hostname;
  String deviceId;
  bool apMode = false;
  bool wifiConnected = false;
  PCState pcState = PCState::OFF;
  bool powerRelayActive = false;
  bool resetRelayActive = false;
  float temperature = 0.0f;
  uint32_t lastHddActiveMs = 0;
  uint32_t freeHeap = 0;
  uint32_t totalHeap = 0;
  uint8_t cpuLoad = 0;
};

// Global instances (defined in main.cpp)
extern StoredConfig g_config;
extern RuntimeState g_state;
