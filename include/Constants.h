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
  // WiFi
  String wifiSsid;
  String wifiPass;
  
  // MQTT Integration
  String mqttHost;
  uint16_t mqttPort = 1883;
  String mqttUser;
  String mqttPass;
  bool mqttTls = false;
  
  // Loki Integration (log aggregation)
  String lokiHost;                // e.g., "http://loki.local:3100"
  String lokiUser;                // Optional basic auth
  String lokiPass;
  
  // Timing
  uint32_t powerPulseMs = 500;
  uint32_t resetPulseMs = 500;
  uint32_t bootGraceMs = 60000;
  
  // Security
  String adminPassword;
};

// Current runtime status (not persistent)
struct RuntimeState {
  String hostname;
  String deviceId;
  String apPassword;              // Unique per-device AP password (EU CRA compliant)
  String defaultAdminPassword;    // Generated default admin password (shown on first boot)
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
  uint8_t authFailCount = 0;      // Rate limiting: failed auth attempts
  uint32_t authBlockedUntilMs = 0; // Rate limiting: blocked until this time
};

// Global instances (defined in main.cpp)
extern StoredConfig g_config;
extern RuntimeState g_state;
