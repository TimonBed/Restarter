/**
 * =============================================================================
 * WebInterface.cpp - Web Server & REST API
 * =============================================================================
 * 
 * This file implements the web interface:
 *   - Serves the web UI files (HTML, CSS, JS) from LittleFS
 *   - Provides REST API endpoints for status and control
 *   - WebSocket for real-time status updates
 *   - Captive portal support for setup mode
 * 
 * REST API ENDPOINTS:
 *   GET  /api/status        - Current device status (JSON)
 *   GET  /api/config        - Current configuration (JSON, no passwords)
 *   POST /api/config        - Save new configuration (restarts device)
 *   POST /api/action/power  - Trigger power button press
 *   POST /api/action/reset  - Trigger reset button press
 *   POST /api/action/force-power - Force shutdown (11s hold)
 *   GET  /api/wifi/scan     - Scan for WiFi networks
 *   POST /api/factory-reset - Clear all settings, restart in AP mode
 * 
 * WEBSOCKET:
 *   /ws - Real-time status updates and action logs
 * 
 * =============================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#include "Config.h"
#include "Constants.h"
#include "PCController.h"

// Global objects defined in main.cpp
extern AsyncWebServer g_server;
extern AsyncWebSocket g_ws;
extern PCController g_pc;
extern StoredConfig g_config;
extern RuntimeState g_state;
extern bool g_restartPending;
extern uint32_t g_restartAtMs;

// External functions from Networking.cpp
bool Networking_saveConfig(const StoredConfig &cfg);
bool Networking_clearConfig();

// =============================================================================
// SECURITY: CSRF PROTECTION
// =============================================================================

// CSRF token expires after 1 hour
constexpr uint32_t CSRF_TOKEN_VALIDITY_MS = 3600000;

static String g_csrfToken;
static uint32_t g_csrfTokenCreatedMs = 0;

static String generateCsrfToken() {
  /**
   * Generate a new CSRF token.
   * Uses device ID + timestamp + random for uniqueness.
   */
  uint32_t rnd = esp_random();
  uint32_t time = millis();
  
  // Simple hash combining device ID, time, and random
  uint32_t hash = 0x1F3E5D7C;
  for (size_t i = 0; i < g_state.deviceId.length(); i++) {
    hash = ((hash << 5) + hash) ^ g_state.deviceId[i];
  }
  hash ^= time;
  hash ^= rnd;
  hash = ((hash << 13) | (hash >> 19)) ^ (rnd >> 7);
  
  // Convert to hex string
  char token[17];
  snprintf(token, sizeof(token), "%08X%08X", hash, rnd);
  return String(token);
}

static String getCsrfToken() {
  /**
   * Get current CSRF token, generating a new one if expired or missing.
   */
  uint32_t now = millis();
  
  // Generate new token if missing or expired
  if (g_csrfToken.length() == 0 || 
      (now - g_csrfTokenCreatedMs) > CSRF_TOKEN_VALIDITY_MS) {
    g_csrfToken = generateCsrfToken();
    g_csrfTokenCreatedMs = now;
    Serial.println("CSRF: Generated new token");
  }
  
  return g_csrfToken;
}

static bool validateCsrfToken(AsyncWebServerRequest *request) {
  /**
   * Validate CSRF token from request header or body.
   * 
   * Token can be provided via:
   *   - X-CSRF-Token header
   *   - csrf_token query parameter
   * 
   * @return true if token is valid
   */
  // AP mode doesn't require CSRF (initial setup)
  if (g_state.apMode) {
    return true;
  }
  
  // Check header first
  if (request->hasHeader("X-CSRF-Token")) {
    String token = request->header("X-CSRF-Token");
    if (token == g_csrfToken) {
      return true;
    }
  }
  
  // Check query parameter
  if (request->hasParam("csrf_token")) {
    String token = request->getParam("csrf_token")->value();
    if (token == g_csrfToken) {
      return true;
    }
  }
  
  Serial.println("CSRF: Token validation failed");
  request->send(403, "application/json", "{\"error\":\"CSRF token invalid or missing\"}");
  return false;
}

// =============================================================================
// SECURITY: AUTHENTICATION & RATE LIMITING
// =============================================================================

// Rate limiting constants
constexpr uint8_t MAX_AUTH_FAILURES = 5;       // Max failed attempts before lockout
constexpr uint32_t AUTH_LOCKOUT_MS = 300000;   // 5 minute lockout
constexpr uint32_t AUTH_FAILURE_DECAY_MS = 60000; // Failures decay after 1 minute

static uint32_t g_lastAuthFailMs = 0;

static bool isRateLimited() {
  /**
   * Check if authentication is currently rate-limited.
   * Returns true if too many failed attempts recently.
   */
  if (g_state.authBlockedUntilMs > 0 && millis() < g_state.authBlockedUntilMs) {
    return true;
  }
  // Reset block if time has passed
  if (g_state.authBlockedUntilMs > 0 && millis() >= g_state.authBlockedUntilMs) {
    g_state.authBlockedUntilMs = 0;
    g_state.authFailCount = 0;
  }
  return false;
}

static void recordAuthFailure() {
  /**
   * Record a failed authentication attempt.
   * Triggers lockout after MAX_AUTH_FAILURES.
   */
  // Decay old failures
  if (millis() - g_lastAuthFailMs > AUTH_FAILURE_DECAY_MS) {
    g_state.authFailCount = 0;
  }
  
  g_lastAuthFailMs = millis();
  g_state.authFailCount++;
  
  if (g_state.authFailCount >= MAX_AUTH_FAILURES) {
    g_state.authBlockedUntilMs = millis() + AUTH_LOCKOUT_MS;
    Serial.printf("AUTH: Too many failures, locked out for %d seconds\n", AUTH_LOCKOUT_MS / 1000);
  }
}

static bool checkAuth(AsyncWebServerRequest *request) {
  /**
   * Verify HTTP Basic Auth credentials.
   * In AP mode (setup), authentication is not required.
   * 
   * @return true if authenticated or in AP mode
   */
  // AP mode doesn't require auth (user already knows AP password)
  if (g_state.apMode) {
    return true;
  }
  
  // Check rate limiting
  if (isRateLimited()) {
    request->send(429, "application/json", "{\"error\":\"Too many attempts. Try again later.\"}");
    return false;
  }
  
  // Check for Authorization header
  if (!request->authenticate("admin", g_config.adminPassword.c_str())) {
    recordAuthFailure();
    request->requestAuthentication("Restarter");
    return false;
  }
  
  // Success - reset failure count
  g_state.authFailCount = 0;
  return true;
}

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

static const char *pcStateToString(PCState state) {
  /**
   * Convert PCState enum to a human-readable string.
   * Used in JSON responses for the web UI.
   */
  switch (state) {
    case PCState::OFF:        return "OFF";
    case PCState::BOOTING:    return "BOOTING";
    case PCState::RUNNING:    return "RUNNING";
    case PCState::RESTARTING: return "RESTARTING";
    default:                  return "UNKNOWN";
  }
}

static String buildStatusJson() {
  /**
   * Build a JSON object containing all current status information.
   * This is sent to:
   *   - GET /api/status endpoint
   *   - WebSocket clients on connect and periodically
   */
  StaticJsonDocument<512> doc;
  
  // Message type (helps UI distinguish status from logs)
  doc["type"] = "status";
  
  // Device identity
  doc["hostname"] = g_state.hostname;
  doc["deviceId"] = g_state.deviceId;
  
  // Network status
  doc["apMode"] = g_state.apMode;
  doc["wifiConnected"] = g_state.wifiConnected;
  doc["hasConfig"] = g_config.wifiSsid.length() > 0;
  
  // PC status
  doc["pcState"] = pcStateToString(g_state.pcState);
  doc["powerRelayActive"] = g_state.powerRelayActive;
  doc["resetRelayActive"] = g_state.resetRelayActive;
  doc["temperature"] = g_state.temperature;
  
  // HDD activity (seconds since last activity, -1 if never seen)
  if (g_state.lastHddActiveMs > 0) {
    doc["hddLastActiveSec"] = (millis() - g_state.lastHddActiveMs) / 1000;
  } else {
    doc["hddLastActiveSec"] = -1;
  }
  
  // ESP32 system stats
  doc["freeHeap"] = g_state.freeHeap;
  doc["totalHeap"] = g_state.totalHeap;
  doc["cpuLoad"] = g_state.cpuLoad;
  
  // WiFi details (different in AP vs STA mode)
  if (g_state.apMode) {
    doc["ssid"] = String(Config::AP_SSID_PREFIX) + g_state.deviceId.substring(6);
    doc["apPassword"] = g_state.apPassword;
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
  
  // CSRF token for API requests (only in STA mode)
  if (!g_state.apMode) {
    doc["csrfToken"] = getCsrfToken();
  }
  
  String out;
  serializeJson(doc, out);
  return out;
}

// =============================================================================
// ACTION LOG (circular buffer)
// =============================================================================
// Keeps recent action logs in memory so new WebSocket clients can see history.

static String g_logBuffer[20];          // Circular buffer of log messages
static size_t g_logCount = 0;           // Number of logs stored
static size_t g_logIndex = 0;           // Next write position
static constexpr size_t kLogBufferSize = sizeof(g_logBuffer) / sizeof(g_logBuffer[0]);

void WebInterface_logAction(const char *message) {
  /**
   * Log an action and broadcast to all connected WebSocket clients.
   * 
   * @param message  Human-readable description of the action
   */
  Serial.print("ACTION: ");
  Serial.println(message);
  
  // Build JSON log message
  StaticJsonDocument<192> doc;
  doc["type"] = "log";
  doc["message"] = message;
  doc["timestampMs"] = millis();
  
  String out;
  serializeJson(doc, out);
  
  // Broadcast to all WebSocket clients
  g_ws.textAll(out);

  // Store in circular buffer for new clients
  g_logBuffer[g_logIndex] = out;
  g_logIndex = (g_logIndex + 1) % kLogBufferSize;
  if (g_logCount < kLogBufferSize) {
    g_logCount++;
  }
}

void WebInterface_broadcastStatus() {
  /**
   * Send current status to all connected WebSocket clients.
   * Called periodically (every STATUS_BROADCAST_MS) from main loop.
   */
  g_ws.textAll(buildStatusJson());
}

// =============================================================================
// SETUP - Register all endpoints
// =============================================================================

void WebInterface_setup() {
  /**
   * Initialize the web server with all endpoints.
   * Called once during setup().
   */
  
  // -------------------------------------------------------------------------
  // WebSocket Handler
  // -------------------------------------------------------------------------
  g_ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client,
                  AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.printf("WebSocket client #%u connected\n", client->id());
      
      // Send current status immediately on connect
      client->text(buildStatusJson());
      
      // Send recent log history
      size_t start = (g_logIndex + kLogBufferSize - g_logCount) % kLogBufferSize;
      for (size_t i = 0; i < g_logCount; i++) {
        size_t idx = (start + i) % kLogBufferSize;
        client->text(g_logBuffer[idx]);
      }
    }
  });
  g_server.addHandler(&g_ws);

  // -------------------------------------------------------------------------
  // Captive Portal Handler
  // -------------------------------------------------------------------------
  // Serves the appropriate page based on mode (onboarding in AP, dashboard in STA)
  auto servePortal = [](AsyncWebServerRequest *request) {
    const char* file = g_state.apMode ? "/onboarding.html" : "/index.html";
    if (LittleFS.exists(file)) {
      AsyncWebServerResponse *response = request->beginResponse(LittleFS, file, "text/html");
      response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
      request->send(response);
    } else {
      request->send(200, "text/plain", "UI not uploaded. Run: pio run --target uploadfs");
    }
  };

  // -------------------------------------------------------------------------
  // API: GET /api/status
  // -------------------------------------------------------------------------
  // Returns current device status as JSON
  g_server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", buildStatusJson());
  });

  // -------------------------------------------------------------------------
  // API: GET /api/config (PROTECTED)
  // -------------------------------------------------------------------------
  // Returns current configuration (passwords are hidden)
  g_server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    
    StaticJsonDocument<512> doc;
    
    // WiFi (password hidden)
    doc["wifiSsid"] = g_config.wifiSsid;
    doc["hasWifiPass"] = g_config.wifiPass.length() > 0;
    
    // MQTT (password hidden)
    doc["mqttHost"] = g_config.mqttHost;
    doc["mqttPort"] = g_config.mqttPort;
    doc["mqttUser"] = g_config.mqttUser;
    doc["hasMqttPass"] = g_config.mqttPass.length() > 0;
    doc["mqttTls"] = g_config.mqttTls;
    
    // Timing settings
    doc["powerPulseMs"] = g_config.powerPulseMs;
    doc["resetPulseMs"] = g_config.resetPulseMs;
    doc["bootGraceMs"] = g_config.bootGraceMs;
    
    // Security (show if using default or custom password)
    doc["hasCustomAdminPass"] = g_config.adminPassword != g_state.defaultAdminPassword;
    
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

  // -------------------------------------------------------------------------
  // API: POST /api/config (PROTECTED + CSRF in STA mode)
  // -------------------------------------------------------------------------
  // Save new configuration and restart
  auto *jsonHandler = new AsyncCallbackJsonWebHandler(
      "/api/config", [](AsyncWebServerRequest *request, JsonVariant &json) {
        // Auth check (not required in AP mode for initial setup)
        if (!checkAuth(request)) return;
        
        JsonObject obj = json.as<JsonObject>();
        
        // CSRF validation (check header or JSON body)
        if (!g_state.apMode) {
          String csrfFromBody = obj["csrfToken"] | "";
          bool headerValid = request->hasHeader("X-CSRF-Token") && 
                            request->header("X-CSRF-Token") == g_csrfToken;
          bool bodyValid = csrfFromBody == g_csrfToken;
          
          if (!headerValid && !bodyValid) {
            Serial.println("CSRF: Token validation failed (config)");
            request->send(403, "application/json", "{\"error\":\"CSRF token invalid\"}");
            return;
          }
        }
        
        StoredConfig cfg;
        
        // WiFi settings (SSID required)
        cfg.wifiSsid = obj["wifiSsid"] | "";
        if (cfg.wifiSsid.length() == 0) {
          request->send(400, "application/json", "{\"error\":\"wifiSsid required\"}");
          return;
        }
        
        // Password handling: preserve existing if not provided
        String newWifiPass = obj["wifiPass"] | "";
        cfg.wifiPass = newWifiPass.length() > 0 ? newWifiPass : g_config.wifiPass;
        
        // MQTT settings (all optional)
        cfg.mqttHost = obj["mqttHost"] | "";
        cfg.mqttPort = obj["mqttPort"] | 1883;
        cfg.mqttUser = obj["mqttUser"] | "";
        String newMqttPass = obj["mqttPass"] | "";
        cfg.mqttPass = newMqttPass.length() > 0 ? newMqttPass : g_config.mqttPass;
        cfg.mqttTls = obj["mqttTls"] | false;
        
        // Timing settings
        cfg.powerPulseMs = obj["powerPulseMs"] | 500;
        cfg.resetPulseMs = obj["resetPulseMs"] | 500;
        cfg.bootGraceMs = obj["bootGraceMs"] | 60000;
        
        // Admin password: preserve existing if not provided
        String newAdminPass = obj["adminPassword"] | "";
        cfg.adminPassword = newAdminPass.length() > 0 ? newAdminPass : g_config.adminPassword;

        // Save to NVS
        if (!Networking_saveConfig(cfg)) {
          request->send(500, "application/json", "{\"error\":\"save failed\"}");
          return;
        }

        // Schedule restart to apply new settings
        g_restartPending = true;
        g_restartAtMs = millis() + 1000;
        request->send(200, "application/json", "{\"ok\":true}");
      });
  g_server.addHandler(jsonHandler);

  // -------------------------------------------------------------------------
  // API: POST /api/action/power (PROTECTED + CSRF)
  // -------------------------------------------------------------------------
  // Trigger a power button press
  g_server.on("/api/action/power", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    if (!validateCsrfToken(request)) return;
    g_pc.pulsePower();
    WebInterface_logAction("Power pulse requested (API)");
    request->send(200, "application/json", "{\"ok\":true}");
  });

  // -------------------------------------------------------------------------
  // API: POST /api/action/reset (PROTECTED + CSRF)
  // -------------------------------------------------------------------------
  // Trigger a reset button press
  g_server.on("/api/action/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    if (!validateCsrfToken(request)) return;
    g_pc.pulseReset();
    WebInterface_logAction("Reset pulse requested (API)");
    request->send(200, "application/json", "{\"ok\":true}");
  });

  // -------------------------------------------------------------------------
  // API: POST /api/action/force-power (PROTECTED + CSRF)
  // -------------------------------------------------------------------------
  // Force shutdown (hold power for 11 seconds)
  g_server.on("/api/action/force-power", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    if (!validateCsrfToken(request)) return;
    g_pc.forcePower();
    WebInterface_logAction("Force shutdown requested (11s hold)");
    request->send(200, "application/json", "{\"ok\":true}");
  });

  // -------------------------------------------------------------------------
  // API: GET /api/wifi/scan
  // -------------------------------------------------------------------------
  // Scan for available WiFi networks (for onboarding wizard)
  g_server.on("/api/wifi/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
    int n = WiFi.scanComplete();
    
    if (n == WIFI_SCAN_FAILED) {
      // Start async scan
      WiFi.scanNetworks(true);
      request->send(202, "application/json", "{\"scanning\":true}");
      return;
    }
    
    if (n == WIFI_SCAN_RUNNING) {
      request->send(202, "application/json", "{\"scanning\":true}");
      return;
    }
    
    // Scan complete - build results
    StaticJsonDocument<1024> doc;
    JsonArray networks = doc.createNestedArray("networks");
    
    for (int i = 0; i < n && i < 15; i++) {
      JsonObject net = networks.createNestedObject();
      net["ssid"] = WiFi.SSID(i);
      net["rssi"] = WiFi.RSSI(i);
      net["secure"] = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
    }
    
    WiFi.scanDelete();  // Clear results to allow new scan
    
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

  // -------------------------------------------------------------------------
  // API: POST /api/factory-reset (PROTECTED + CSRF)
  // -------------------------------------------------------------------------
  // Clear all configuration and restart in AP mode
  g_server.on("/api/factory-reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    if (!validateCsrfToken(request)) return;
    WebInterface_logAction("Factory reset initiated");
    Networking_clearConfig();
    g_restartPending = true;
    g_restartAtMs = millis() + 500;
    request->send(200, "application/json", "{\"ok\":true}");
  });

  // -------------------------------------------------------------------------
  // Captive Portal Probes
  // -------------------------------------------------------------------------
  // These endpoints are checked by various operating systems to detect
  // captive portals. By responding, we trigger the portal popup.
  g_server.on("/generate_204", HTTP_GET, servePortal);       // Android
  g_server.on("/fwlink", HTTP_GET, servePortal);             // Windows
  g_server.on("/hotspot-detect.html", HTTP_GET, servePortal); // iOS
  g_server.on("/connecttest.txt", HTTP_GET, servePortal);    // Windows

  // -------------------------------------------------------------------------
  // Static Files
  // -------------------------------------------------------------------------
  // Serve all files from LittleFS with no-cache headers
  g_server.serveStatic("/", LittleFS, "/")
      .setCacheControl("no-cache, no-store, must-revalidate");
  
  // -------------------------------------------------------------------------
  // Root Path
  // -------------------------------------------------------------------------
  // Serve onboarding wizard in AP mode, dashboard in STA mode
  g_server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    const char* file = g_state.apMode ? "/onboarding.html" : "/index.html";
    if (LittleFS.exists(file)) {
      AsyncWebServerResponse *response = request->beginResponse(LittleFS, file, "text/html");
      response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
      request->send(response);
    } else {
      request->send(200, "text/plain", "UI not uploaded. Run: pio run --target uploadfs");
    }
  });
  
  // -------------------------------------------------------------------------
  // 404 Handler
  // -------------------------------------------------------------------------
  g_server.onNotFound([](AsyncWebServerRequest *request) {
    // In AP mode, redirect everything to root (captive portal behavior)
    if (g_state.apMode) {
      request->redirect("/");
      return;
    }
    
    // In STA mode, serve index.html for SPA routing (or 404)
    if (LittleFS.exists("/index.html")) {
      AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html", "text/html");
      response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
      request->send(response);
    } else {
      request->send(404, "text/plain", "Not found");
    }
  });
  
  // Start the server!
  g_server.begin();
  Serial.println("Web server started on port 80");
}

// =============================================================================
// LOOP - Periodic maintenance
// =============================================================================

void WebInterface_loop() {
  /**
   * Handle ongoing web server tasks.
   * Called continuously in main loop().
   */
  
  // Clean up disconnected WebSocket clients
  g_ws.cleanupClients();
}
