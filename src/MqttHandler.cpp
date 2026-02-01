/**
 * =============================================================================
 * MqttHandler.cpp - MQTT & Home Assistant Integration
 * =============================================================================
 * 
 * This file handles MQTT connectivity for Home Assistant integration:
 *   - Auto-discovery (Home Assistant finds the device automatically)
 *   - Power switch entity (turn PC on/off)
 *   - Reset button entity (trigger reset)
 *   - Status publishing (PC state, WiFi status)
 * 
 * MQTT TOPIC STRUCTURE:
 *   restarter/<deviceId>/
 *   ├── availability     "online" or "offline" (Last Will)
 *   ├── power/
 *   │   ├── set          Command topic (send any message to toggle)
 *   │   └── state        State topic ("ON" or "OFF")
 *   ├── reset/
 *   │   └── press        Command topic (send "PRESS" to trigger)
 *   └── status           JSON with full status
 * 
 * HOME ASSISTANT AUTO-DISCOVERY:
 *   Discovery messages are published to:
 *   - homeassistant/switch/<deviceId>/power/config
 *   - homeassistant/button/<deviceId>/reset/config
 * 
 * =============================================================================
 */

#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "Config.h"
#include "Constants.h"
#include "PCController.h"

// Global objects from main.cpp
extern PubSubClient g_mqttClient;
extern StoredConfig g_config;
extern RuntimeState g_state;
extern PCController g_pc;

// External function from WebInterface.cpp (for logging actions)
void WebInterface_logAction(const char *message);

// =============================================================================
// TOPIC HELPERS
// =============================================================================
// These functions build topic strings based on the device ID.
// Example for device "a1b2c3d4e5f6":
//   baseTopic() = "restarter/a1b2c3d4e5f6"

static String baseTopic() {
  return String("restarter/") + g_state.deviceId;
}

static String availabilityTopic() {
  // Published when connecting ("online") and as Last Will ("offline")
  return baseTopic() + "/availability";
}

static String powerCommandTopic() {
  // Subscribe: receive power toggle commands
  return baseTopic() + "/power/set";
}

static String powerStateTopic() {
  // Publish: current power state ("ON" or "OFF")
  return baseTopic() + "/power/state";
}

static String resetCommandTopic() {
  // Subscribe: receive reset button commands
  return baseTopic() + "/reset/press";
}

static String statusTopic() {
  // Publish: full JSON status
  return baseTopic() + "/status";
}

// =============================================================================
// HOME ASSISTANT AUTO-DISCOVERY
// =============================================================================

static void publishDiscovery() {
  /**
   * Publish Home Assistant MQTT discovery messages.
   * 
   * This allows Home Assistant to automatically discover the device
   * and create the appropriate entities without manual configuration.
   * 
   * We create two entities:
   *   1. Switch: "Power" - toggle to pulse the power button
   *   2. Button: "Reset" - press to pulse the reset button
   */
  Serial.println("Publishing Home Assistant discovery...");
  
  // -------------------------------------------------------------------------
  // Device Information (shared by all entities)
  // -------------------------------------------------------------------------
  StaticJsonDocument<512> device;
  device["ids"][0] = g_state.deviceId;                      // Unique device ID
  device["name"] = String("Restarter ") + g_state.deviceId; // Display name
  device["mf"] = "Restarter";                                // Manufacturer
  device["mdl"] = "ESP32-C3 Restarter";                      // Model
  device["sw"] = Config::FW_VERSION;                         // Firmware version

  // -------------------------------------------------------------------------
  // Power Switch Entity
  // -------------------------------------------------------------------------
  // This appears as a switch in Home Assistant that can be toggled
  StaticJsonDocument<768> sw;
  sw["name"] = "Power";
  sw["uniq_id"] = g_state.deviceId + String("_power");
  sw["cmd_t"] = powerCommandTopic();    // Command topic (to receive commands)
  sw["stat_t"] = powerStateTopic();     // State topic (to report state)
  sw["avty_t"] = availabilityTopic();   // Availability topic
  sw["pl_avail"] = "online";            // Payload when device is online
  sw["pl_not_avail"] = "offline";       // Payload when device is offline
  sw["device"] = device;                // Link to device info

  String swPayload;
  serializeJson(sw, swPayload);
  String swTopic = String("homeassistant/switch/") + g_state.deviceId + "/power/config";
  g_mqttClient.publish(swTopic.c_str(), swPayload.c_str(), true);  // Retained

  // -------------------------------------------------------------------------
  // Reset Button Entity
  // -------------------------------------------------------------------------
  // This appears as a button in Home Assistant that can be pressed
  StaticJsonDocument<768> btn;
  btn["name"] = "Reset";
  btn["uniq_id"] = g_state.deviceId + String("_reset");
  btn["cmd_t"] = resetCommandTopic();
  btn["avty_t"] = availabilityTopic();
  btn["pl_avail"] = "online";
  btn["pl_not_avail"] = "offline";
  btn["payload_press"] = "PRESS";       // What to send when button is pressed
  btn["device"] = device;

  String btnPayload;
  serializeJson(btn, btnPayload);
  String btnTopic = String("homeassistant/button/") + g_state.deviceId + "/reset/config";
  g_mqttClient.publish(btnTopic.c_str(), btnPayload.c_str(), true);  // Retained
  
  Serial.println("Discovery published successfully");
}

// =============================================================================
// MESSAGE CALLBACK
// =============================================================================

static void mqttCallback(char *topic, byte *payload, unsigned int length) {
  /**
   * Handle incoming MQTT messages.
   * 
   * Called when we receive a message on a subscribed topic.
   * We check which topic it came from and trigger the appropriate action.
   */
  String topicStr(topic);
  
  Serial.print("MQTT message on topic: ");
  Serial.println(topicStr);
  
  if (topicStr == powerCommandTopic()) {
    // Power command received - pulse the power button
    g_pc.pulsePower();
    WebInterface_logAction("Power pulse requested (MQTT)");
  } else if (topicStr == resetCommandTopic()) {
    // Reset command received - pulse the reset button
    g_pc.pulseReset();
    WebInterface_logAction("Reset pulse requested (MQTT)");
  }
}

// =============================================================================
// CONNECTION
// =============================================================================

static bool connectMqtt() {
  /**
   * Connect to the MQTT broker.
   * 
   * Uses configuration from g_config:
   *   - mqttHost: Broker address (IP or hostname)
   *   - mqttPort: Broker port (default 1883)
   *   - mqttUser: Username (optional)
   *   - mqttPass: Password (optional)
   * 
   * Also configures:
   *   - Client ID: "restarter-<deviceId>"
   *   - Last Will: Publishes "offline" to availability topic on disconnect
   * 
   * @return true if connected, false otherwise
   */
  if (g_config.mqttHost.length() == 0) {
    // MQTT not configured - skip silently
    return false;
  }
  
  Serial.print("Connecting to MQTT broker: ");
  Serial.print(g_config.mqttHost);
  Serial.print(":");
  Serial.println(g_config.mqttPort);
  
  g_mqttClient.setServer(g_config.mqttHost.c_str(), g_config.mqttPort);
  String clientId = String("restarter-") + g_state.deviceId;

  bool ok = false;
  
  if (g_config.mqttUser.length() > 0) {
    // Connect with authentication
    ok = g_mqttClient.connect(
        clientId.c_str(),
        g_config.mqttUser.c_str(),
        g_config.mqttPass.c_str(),
        availabilityTopic().c_str(),  // Last Will topic
        0,                             // QoS
        true,                          // Retain
        "offline"                      // Last Will message
    );
  } else {
    // Connect without authentication
    ok = g_mqttClient.connect(
        clientId.c_str(),
        availabilityTopic().c_str(),
        0,
        true,
        "offline"
    );
  }

  if (ok) {
    Serial.println("MQTT connected!");
    
    // Publish "online" to availability topic
    g_mqttClient.publish(availabilityTopic().c_str(), "online", true);
    
    // Subscribe to command topics
    g_mqttClient.subscribe(powerCommandTopic().c_str());
    g_mqttClient.subscribe(resetCommandTopic().c_str());
    
    // Publish Home Assistant discovery
    publishDiscovery();
  } else {
    Serial.print("MQTT connection failed, state: ");
    Serial.println(g_mqttClient.state());
  }
  
  return ok;
}

// =============================================================================
// SETUP
// =============================================================================

void MqttHandler_setup() {
  /**
   * Initialize MQTT handler.
   * Called once during setup().
   */
  g_mqttClient.setCallback(mqttCallback);
  Serial.println("MQTT handler initialized");
}

// =============================================================================
// MAIN LOOP
// =============================================================================

void MqttHandler_loop() {
  /**
   * Handle MQTT connection and messages.
   * Called continuously in main loop().
   * 
   * This function:
   *   - Skips if WiFi is not connected or MQTT is not configured
   *   - Processes incoming messages if connected
   *   - Attempts reconnection if disconnected (with rate limiting)
   */
  static uint32_t lastAttemptMs = 0;
  
  // Skip if WiFi not connected or MQTT not configured
  if (!g_state.wifiConnected || g_config.mqttHost.length() == 0) {
    return;
  }
  
  // If connected, process the message loop
  if (g_mqttClient.connected()) {
    g_mqttClient.loop();
    return;
  }
  
  // Not connected - try to reconnect (with rate limiting)
  if (millis() - lastAttemptMs > Config::MQTT_RECONNECT_MS) {
    lastAttemptMs = millis();
    connectMqtt();
  }
}

// =============================================================================
// STATE PUBLISHING
// =============================================================================

void MqttHandler_publishState() {
  /**
   * Publish current state to MQTT.
   * Called periodically from main loop.
   * 
   * Publishes:
   *   - Power state to power/state topic ("ON" or "OFF")
   *   - Full status JSON to status topic
   */
  if (!g_mqttClient.connected()) {
    return;
  }
  
  // Publish power state (for the Home Assistant switch)
  const char *powerState = (g_state.pcState == PCState::OFF) ? "OFF" : "ON";
  g_mqttClient.publish(powerStateTopic().c_str(), powerState, true);

  // Publish full status JSON (for dashboards and automation)
  StaticJsonDocument<256> doc;
  doc["pcState"] = static_cast<uint8_t>(g_state.pcState);
  doc["pcStateName"] = (g_state.pcState == PCState::OFF) ? "OFF" :
                       (g_state.pcState == PCState::BOOTING) ? "BOOTING" :
                       (g_state.pcState == PCState::RUNNING) ? "RUNNING" : "RESTARTING";
  doc["wifiConnected"] = g_state.wifiConnected;
  
  String payload;
  serializeJson(doc, payload);
  g_mqttClient.publish(statusTopic().c_str(), payload.c_str(), false);
}
