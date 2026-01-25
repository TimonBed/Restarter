#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "Config.h"
#include "Constants.h"

extern StoredConfig g_config;
extern RuntimeState g_state;

static DNSServer g_dnsServer; // Captive portal DNS for AP mode

static String formatDeviceId(uint64_t mac) {
  // Turn MAC into a short, stable ID string.
  char buf[13] = {0};
  snprintf(buf, sizeof(buf), "%02x%02x%02x%02x%02x%02x",
           (uint8_t)(mac >> 40), (uint8_t)(mac >> 32),
           (uint8_t)(mac >> 24), (uint8_t)(mac >> 16),
           (uint8_t)(mac >> 8), (uint8_t)(mac));
  return String(buf);
}

static void loadIdentity() {
  // Use MAC to build a unique hostname (no default passwords).
  uint64_t mac = ESP.getEfuseMac();
  g_state.deviceId = formatDeviceId(mac);
  g_state.hostname = String(Config::HOSTNAME_PREFIX) + g_state.deviceId;
}

bool Networking_hasConfig() {
  // WiFi SSID present means user configured WiFi.
  return g_config.wifiSsid.length() > 0;
}

bool Networking_loadConfig() {
  // Read saved config from LittleFS.
  if (!LittleFS.exists(Config::CONFIG_PATH)) {
    return false;
  }
  File file = LittleFS.open(Config::CONFIG_PATH, "r");
  if (!file) {
    return false;
  }
  StaticJsonDocument<512> doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();
  if (err) {
    return false;
  }

  g_config.wifiSsid = doc["wifiSsid"] | "";
  g_config.wifiPass = doc["wifiPass"] | "";
  g_config.mqttHost = doc["mqttHost"] | "";
  g_config.mqttPort = doc["mqttPort"] | 1883;
  g_config.mqttUser = doc["mqttUser"] | "";
  g_config.mqttPass = doc["mqttPass"] | "";
  return Networking_hasConfig();
}

bool Networking_saveConfig(const StoredConfig &cfg) {
  // Save config to LittleFS so it persists across reboots.
  StaticJsonDocument<512> doc;
  doc["wifiSsid"] = cfg.wifiSsid;
  doc["wifiPass"] = cfg.wifiPass;
  doc["mqttHost"] = cfg.mqttHost;
  doc["mqttPort"] = cfg.mqttPort;
  doc["mqttUser"] = cfg.mqttUser;
  doc["mqttPass"] = cfg.mqttPass;

  File file = LittleFS.open(Config::CONFIG_PATH, "w");
  if (!file) {
    return false;
  }
  if (serializeJson(doc, file) == 0) {
    file.close();
    return false;
  }
  file.close();
  g_config = cfg;
  return true;
}

static bool connectSta() {
  // Connect to a WiFi router using stored credentials.
  if (!Networking_hasConfig()) {
    return false;
  }
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(g_state.hostname.c_str());
  WiFi.begin(g_config.wifiSsid.c_str(), g_config.wifiPass.c_str());

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < Config::WIFI_CONNECT_TIMEOUT_MS) {
    delay(50);
  }
  g_state.wifiConnected = (WiFi.status() == WL_CONNECTED);
  return g_state.wifiConnected;
}

static void startAp() {
  // Start Access Point for first-time setup.
  WiFi.mode(WIFI_AP);
  String ssid = String(Config::AP_SSID_PREFIX) + g_state.deviceId.substring(6);
  WiFi.softAP(ssid.c_str());
  g_dnsServer.start(53, "*", WiFi.softAPIP());
  g_state.apMode = true;
  g_state.wifiConnected = false;
}

void Networking_setup() {
  // Identity + storage + WiFi selection.
  loadIdentity();
  if (!LittleFS.begin(true)) {
    // LittleFS failed; continue without storage
  }
  Networking_loadConfig();

  if (!connectSta()) {
    startAp();
  } else {
    g_state.apMode = false;
  }
}

void Networking_loop() {
  // Keep WiFi and captive portal alive.
  static uint32_t lastAttemptMs = 0;
  static uint32_t apStartMs = millis();

  if (g_state.apMode) {
    // DNS redirect so phones open the portal.
    g_dnsServer.processNextRequest();
    if (millis() - apStartMs > Config::AP_IDLE_TIMEOUT_MS && Networking_hasConfig()) {
      ESP.restart();
    }
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    g_state.wifiConnected = true;
    return;
  }
  g_state.wifiConnected = false;

  if (millis() - lastAttemptMs > Config::WIFI_CONNECT_TIMEOUT_MS) {
    lastAttemptMs = millis();
    connectSta();
  }
}
