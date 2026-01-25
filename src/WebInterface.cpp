#include <Arduino.h>
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
  StaticJsonDocument<256> doc;
  doc["hostname"] = g_state.hostname;
  doc["deviceId"] = g_state.deviceId;
  doc["apMode"] = g_state.apMode;
  doc["wifiConnected"] = g_state.wifiConnected;
  doc["hasConfig"] = g_config.wifiSsid.length() > 0;
  doc["pcState"] = pcStateToString(g_state.pcState);
  doc["powerRelayActive"] = g_state.powerRelayActive;
  doc["resetRelayActive"] = g_state.resetRelayActive;
  String out;
  serializeJson(doc, out);
  return out;
}

void WebInterface_broadcastStatus() {
  // Push status to all connected WebSocket clients.
  g_ws.textAll(buildStatusJson());
}

void WebInterface_setup() {
  // WebSocket + HTTP endpoints + static UI.
  g_ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client,
                  AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      // Send initial snapshot on connect.
      client->text(buildStatusJson());
    }
  });
  g_server.addHandler(&g_ws);

  auto servePortal = [](AsyncWebServerRequest *request) {
    // Serve the UI directly (avoid redirect loops).
    if (LittleFS.exists("/index.html")) {
      request->send(LittleFS, "/index.html", "text/html");
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
    StaticJsonDocument<256> doc;
    doc["wifiSsid"] = g_config.wifiSsid;
    doc["mqttHost"] = g_config.mqttHost;
    doc["mqttPort"] = g_config.mqttPort;
    doc["mqttUser"] = g_config.mqttUser;
    doc["hasWifiPass"] = g_config.wifiPass.length() > 0;
    doc["hasMqttPass"] = g_config.mqttPass.length() > 0;
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
        cfg.wifiPass = obj["wifiPass"] | "";
        cfg.mqttHost = obj["mqttHost"] | "";
        cfg.mqttPort = obj["mqttPort"] | 1883;
        cfg.mqttUser = obj["mqttUser"] | "";
        cfg.mqttPass = obj["mqttPass"] | "";

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
    request->send(200, "application/json", "{\"ok\":true}");
  });

  g_server.on("/api/action/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Pulse reset relay.
    g_pc.pulseReset();
    request->send(200, "application/json", "{\"ok\":true}");
  });

  // Captive-portal probe endpoints for Android/iOS/Windows.
  g_server.on("/generate_204", HTTP_GET, servePortal);
  g_server.on("/fwlink", HTTP_GET, servePortal);
  g_server.on("/hotspot-detect.html", HTTP_GET, servePortal);
  g_server.on("/connecttest.txt", HTTP_GET, servePortal);

  // Serve the UI and route unknown pages to the portal.
  g_server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
  g_server.onNotFound([](AsyncWebServerRequest *request) {
    if (LittleFS.exists("/index.html")) {
      request->send(LittleFS, "/index.html", "text/html");
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
