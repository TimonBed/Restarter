#pragma once

#include <Arduino.h>

enum class PCState : uint8_t {
  OFF = 0,
  BOOTING,
  RUNNING,
  RESTARTING,
};

struct StoredConfig {
  String wifiSsid;
  String wifiPass;
  String mqttHost;
  uint16_t mqttPort = 1883;
  String mqttUser;
  String mqttPass;
};

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
