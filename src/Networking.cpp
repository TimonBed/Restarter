#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <Preferences.h>

#include "Config.h"
#include "Constants.h"

extern StoredConfig g_config;
extern RuntimeState g_state;

static DNSServer g_dnsServer; // Captive portal DNS for AP mode
static Preferences g_prefs;   // NVS storage - survives LittleFS uploads

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
  // Read config from NVS (survives LittleFS uploads).
  g_prefs.begin("restarter", true); // read-only
  g_config.wifiSsid = g_prefs.getString("wifiSsid", "");
  g_config.wifiPass = g_prefs.getString("wifiPass", "");
  g_config.mqttHost = g_prefs.getString("mqttHost", "");
  g_config.mqttPort = g_prefs.getUShort("mqttPort", 1883);
  g_config.mqttUser = g_prefs.getString("mqttUser", "");
  g_config.mqttPass = g_prefs.getString("mqttPass", "");
  g_config.powerPulseMs = g_prefs.getULong("powerPulseMs", 500);
  g_config.resetPulseMs = g_prefs.getULong("resetPulseMs", 250);
  g_config.bootGraceMs = g_prefs.getULong("bootGraceMs", 60000);
  g_prefs.end();
  return Networking_hasConfig();
}

bool Networking_saveConfig(const StoredConfig &cfg) {
  // Save config to NVS (separate from LittleFS partition).
  g_prefs.begin("restarter", false); // read-write
  g_prefs.putString("wifiSsid", cfg.wifiSsid);
  g_prefs.putString("wifiPass", cfg.wifiPass);
  g_prefs.putString("mqttHost", cfg.mqttHost);
  g_prefs.putUShort("mqttPort", cfg.mqttPort);
  g_prefs.putString("mqttUser", cfg.mqttUser);
  g_prefs.putString("mqttPass", cfg.mqttPass);
  g_prefs.putULong("powerPulseMs", cfg.powerPulseMs);
  g_prefs.putULong("resetPulseMs", cfg.resetPulseMs);
  g_prefs.putULong("bootGraceMs", cfg.bootGraceMs);
  g_prefs.end();
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
