#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#include "Config.h"
#include "Constants.h"
#include "PCController.h"

extern AsyncWebServer g_server;
extern AsyncWebSocket g_ws;
extern PCController g_pc;
extern StoredConfig g_config;
extern RuntimeState g_state;
extern bool g_restartPending;
extern uint32_t g_restartAtMs;

bool Networking_saveConfig(const StoredConfig &cfg);

static const char *pcStateToString(PCState state) {
  // Map enum to a user-friendly string.
  switch (state) {
    case PCState::OFF:
      return "OFF";
    case PCState::BOOTING:
      return "BOOTING";
    case PCState::RUNNING:
      return "RUNNING";
    case PCState::RESTARTING:
      return "RESTARTING";
    default:
      return "UNKNOWN";
  }
}

static String buildStatusJson() {
  // Build a compact status payload for UI + WebSocket.
  StaticJsonDocument<384> doc;
  doc["type"] = "status";
  doc["hostname"] = g_state.hostname;
  doc["deviceId"] = g_state.deviceId;
  doc["apMode"] = g_state.apMode;
  doc["wifiConnected"] = g_state.wifiConnected;
  doc["hasConfig"] = g_config.wifiSsid.length() > 0;
  doc["pcState"] = pcStateToString(g_state.pcState);
  doc["powerRelayActive"] = g_state.powerRelayActive;
  doc["resetRelayActive"] = g_state.resetRelayActive;
  doc["temperature"] = g_state.temperature;
  // HDD last active (seconds ago, 0 if never)
  if (g_state.lastHddActiveMs > 0) {
    doc["hddLastActiveSec"] = (millis() - g_state.lastHddActiveMs) / 1000;
  } else {
    doc["hddLastActiveSec"] = -1; // never seen
  }
  // WiFi details
  if (g_state.apMode) {
    doc["ssid"] = String(Config::AP_SSID_PREFIX) + g_state.deviceId.substring(6);
    doc["ip"] = WiFi.softAPIP().toString();
    doc["rssi"] = 0;
  } else if (g_state.wifiConnected) {
    doc["ssid"] = WiFi.SSID();
    doc["ip"] = WiFi.localIP().toString();
    doc["rssi"] = WiFi.RSSI();
  } else {
    doc["ssid"] = g_config.wifiSsid;
    doc["ip"] = "";
    doc["rssi"] = 0;
  }
  String out;
  serializeJson(doc, out);
  return out;
}

void WebInterface_broadcastStatus() {
  // Push status to all connected WebSocket clients.
  g_ws.textAll(buildStatusJson());
}

static String g_logBuffer[20];
static size_t g_logCount = 0;
static size_t g_logIndex = 0;
static constexpr size_t kLogBufferSize = sizeof(g_logBuffer) / sizeof(g_logBuffer[0]);

void WebInterface_logAction(const char *message) {
  // Send a lightweight log event to all UI clients.
  StaticJsonDocument<192> doc;
  doc["type"] = "log";
  doc["message"] = message;
  doc["timestampMs"] = millis();
  String out;
  serializeJson(doc, out);
  g_ws.textAll(out);

  // Keep a small in-memory log for clients that connect later.
  g_logBuffer[g_logIndex] = out;
  g_logIndex = (g_logIndex + 1) % kLogBufferSize;
  if (g_logCount < kLogBufferSize) {
    g_logCount++;
  }
}

void WebInterface_setup() {
  // WebSocket + HTTP endpoints + static UI.
  g_ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client,
                  AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      // Send initial snapshot on connect.
      client->text(buildStatusJson());
      size_t start = (g_logIndex + kLogBufferSize - g_logCount) % kLogBufferSize;
      for (size_t i = 0; i < g_logCount; i++) {
        size_t idx = (start + i) % kLogBufferSize;
        client->text(g_logBuffer[idx]);
      }
    }
  });
  g_server.addHandler(&g_ws);

  auto servePortal = [](AsyncWebServerRequest *request) {
    // Serve the UI directly (avoid redirect loops), no caching.
    if (LittleFS.exists("/index.html")) {
      AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html", "text/html");
      response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
      request->send(response);
    } else {
      request->send(200, "text/plain", "UI not uploaded. Upload LittleFS data.");
    }
  };

  g_server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Read-only status for UI polling.
    request->send(200, "application/json", buildStatusJson());
  });

  g_server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Return current config (no passwords).
    StaticJsonDocument<384> doc;
    doc["wifiSsid"] = g_config.wifiSsid;
    doc["mqttHost"] = g_config.mqttHost;
    doc["mqttPort"] = g_config.mqttPort;
    doc["mqttUser"] = g_config.mqttUser;
    doc["hasWifiPass"] = g_config.wifiPass.length() > 0;
    doc["hasMqttPass"] = g_config.mqttPass.length() > 0;
    doc["powerPulseMs"] = g_config.powerPulseMs;
    doc["resetPulseMs"] = g_config.resetPulseMs;
    doc["bootGraceMs"] = g_config.bootGraceMs;
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

  auto *jsonHandler = new AsyncCallbackJsonWebHandler(
      "/api/config", [](AsyncWebServerRequest *request, JsonVariant &json) {
        // Save new config and reboot.
        JsonObject obj = json.as<JsonObject>();
        StoredConfig cfg;
        cfg.wifiSsid = obj["wifiSsid"] | "";
        // Preserve existing passwords if not provided
        String newWifiPass = obj["wifiPass"] | "";
        cfg.wifiPass = newWifiPass.length() > 0 ? newWifiPass : g_config.wifiPass;
        cfg.mqttHost = obj["mqttHost"] | "";
        cfg.mqttPort = obj["mqttPort"] | 1883;
        cfg.mqttUser = obj["mqttUser"] | "";
        String newMqttPass = obj["mqttPass"] | "";
        cfg.mqttPass = newMqttPass.length() > 0 ? newMqttPass : g_config.mqttPass;
        cfg.powerPulseMs = obj["powerPulseMs"] | 500;
        cfg.resetPulseMs = obj["resetPulseMs"] | 250;
        cfg.bootGraceMs = obj["bootGraceMs"] | 60000;

        if (cfg.wifiSsid.length() == 0) {
          request->send(400, "application/json", "{\"error\":\"wifiSsid required\"}");
          return;
        }

        if (!Networking_saveConfig(cfg)) {
          request->send(500, "application/json", "{\"error\":\"save failed\"}");
          return;
        }

        g_restartPending = true;
        g_restartAtMs = millis() + 1000;
        request->send(200, "application/json", "{\"ok\":true}");
      });
  g_server.addHandler(jsonHandler);

  g_server.on("/api/action/power", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Pulse power relay.
    g_pc.pulsePower();
    WebInterface_logAction("Power pulse requested (API)");
    request->send(200, "application/json", "{\"ok\":true}");
  });

  g_server.on("/api/action/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Pulse reset relay.
    g_pc.pulseReset();
    WebInterface_logAction("Reset pulse requested (API)");
    request->send(200, "application/json", "{\"ok\":true}");
  });

  g_server.on("/api/wifi/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Scan for available WiFi networks.
    int n = WiFi.scanComplete();
    if (n == WIFI_SCAN_FAILED) {
      WiFi.scanNetworks(true); // Start async scan
      request->send(202, "application/json", "{\"scanning\":true}");
      return;
    }
    if (n == WIFI_SCAN_RUNNING) {
      request->send(202, "application/json", "{\"scanning\":true}");
      return;
    }
    // Build JSON array of networks
    StaticJsonDocument<1024> doc;
    JsonArray networks = doc.createNestedArray("networks");
    for (int i = 0; i < n && i < 15; i++) {
      JsonObject net = networks.createNestedObject();
      net["ssid"] = WiFi.SSID(i);
      net["rssi"] = WiFi.RSSI(i);
      net["secure"] = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
    }
    WiFi.scanDelete(); // Clear results
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

  // Captive-portal probe endpoints for Android/iOS/Windows.
  g_server.on("/generate_204", HTTP_GET, servePortal);
  g_server.on("/fwlink", HTTP_GET, servePortal);
  g_server.on("/hotspot-detect.html", HTTP_GET, servePortal);
  g_server.on("/connecttest.txt", HTTP_GET, servePortal);

  // Serve the UI with no-cache headers (ensures fresh files after LittleFS upload).
  g_server.serveStatic("/", LittleFS, "/")
      .setDefaultFile("index.html")
      .setCacheControl("no-cache, no-store, must-revalidate");
  g_server.onNotFound([](AsyncWebServerRequest *request) {
    if (LittleFS.exists("/index.html")) {
      AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html", "text/html");
      response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
      request->send(response);
    } else {
      request->send(404, "text/plain", "Not found");
    }
  });
  g_server.begin();
}

void WebInterface_loop() {
  // Clean up stale WebSocket clients.
  g_ws.cleanupClients();
}
