#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "Config.h"
#include "Constants.h"
#include "PCController.h"

extern PubSubClient g_mqttClient;
extern StoredConfig g_config;
extern RuntimeState g_state;
extern PCController g_pc;

static String baseTopic() {
  return String("restarter/") + g_state.deviceId;
}

static String availabilityTopic() {
  return baseTopic() + "/availability";
}

static String powerCommandTopic() {
  return baseTopic() + "/power/set";
}

static String powerStateTopic() {
  return baseTopic() + "/power/state";
}

static String resetCommandTopic() {
  return baseTopic() + "/reset/press";
}

static String statusTopic() {
  return baseTopic() + "/status";
}

static void publishDiscovery() {
  StaticJsonDocument<512> device;
  device["ids"][0] = g_state.deviceId;
  device["name"] = String("Restarter ") + g_state.deviceId;
  device["mf"] = "Restarter";
  device["mdl"] = "ESP32-C3 Restarter";
  device["sw"] = Config::FW_VERSION;

  StaticJsonDocument<768> sw;
  sw["name"] = "Power";
  sw["uniq_id"] = g_state.deviceId + String("_power");
  sw["cmd_t"] = powerCommandTopic();
  sw["stat_t"] = powerStateTopic();
  sw["avty_t"] = availabilityTopic();
  sw["pl_avail"] = "online";
  sw["pl_not_avail"] = "offline";
  sw["device"] = device;

  String swPayload;
  serializeJson(sw, swPayload);
  String swTopic = String("homeassistant/switch/") + g_state.deviceId + "/power/config";
  g_mqttClient.publish(swTopic.c_str(), swPayload.c_str(), true);

  StaticJsonDocument<768> btn;
  btn["name"] = "Reset";
  btn["uniq_id"] = g_state.deviceId + String("_reset");
  btn["cmd_t"] = resetCommandTopic();
  btn["avty_t"] = availabilityTopic();
  btn["pl_avail"] = "online";
  btn["pl_not_avail"] = "offline";
  btn["payload_press"] = "PRESS";
  btn["device"] = device;

  String btnPayload;
  serializeJson(btn, btnPayload);
  String btnTopic = String("homeassistant/button/") + g_state.deviceId + "/reset/config";
  g_mqttClient.publish(btnTopic.c_str(), btnPayload.c_str(), true);

}

static void mqttCallback(char *topic, byte *payload, unsigned int length) {
  String topicStr(topic);
  if (topicStr == powerCommandTopic()) {
    g_pc.pulsePower();
  } else if (topicStr == resetCommandTopic()) {
    g_pc.pulseReset();
  }
}

void MqttHandler_setup() {
  g_mqttClient.setCallback(mqttCallback);
}

static bool connectMqtt() {
  if (g_config.mqttHost.length() == 0) {
    return false;
  }
  g_mqttClient.setServer(g_config.mqttHost.c_str(), g_config.mqttPort);
  String clientId = String("restarter-") + g_state.deviceId;

  bool ok = false;
  if (g_config.mqttUser.length() > 0) {
    ok = g_mqttClient.connect(clientId.c_str(),
                              g_config.mqttUser.c_str(),
                              g_config.mqttPass.c_str(),
                              availabilityTopic().c_str(),
                              0,
                              true,
                              "offline");
  } else {
    ok = g_mqttClient.connect(clientId.c_str(),
                              availabilityTopic().c_str(),
                              0,
                              true,
                              "offline");
  }

  if (ok) {
    g_mqttClient.publish(availabilityTopic().c_str(), "online", true);
    g_mqttClient.subscribe(powerCommandTopic().c_str());
    g_mqttClient.subscribe(resetCommandTopic().c_str());
    publishDiscovery();
  }
  return ok;
}

void MqttHandler_loop() {
  static uint32_t lastAttemptMs = 0;
  if (!g_state.wifiConnected || g_config.mqttHost.length() == 0) {
    return;
  }
  if (g_mqttClient.connected()) {
    g_mqttClient.loop();
    return;
  }
  if (millis() - lastAttemptMs > Config::MQTT_RECONNECT_MS) {
    lastAttemptMs = millis();
    connectMqtt();
  }
}

void MqttHandler_publishState() {
  if (!g_mqttClient.connected()) {
    return;
  }
  const char *powerState = (g_state.pcState == PCState::OFF) ? "OFF" : "ON";
  g_mqttClient.publish(powerStateTopic().c_str(), powerState, true);

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
