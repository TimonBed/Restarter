/**
 * =============================================================================
 * LokiHandler.cpp - Grafana Loki Log Shipping
 * =============================================================================
 * 
 * Ships device logs to Grafana Loki for centralized log aggregation.
 * 
 * Configuration:
 *   lokiHost: "http://loki.local:3100" (or https with auth)
 *   lokiUser: Optional basic auth username
 *   lokiPass: Optional basic auth password
 * 
 * Log levels: DEBUG, INFO, WARN, ERROR
 * 
 * =============================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>

#include "Config.h"
#include "Constants.h"
#include "integrations/LokiHandler.h"

// Global objects
extern StoredConfig g_config;
extern RuntimeState g_state;

// =============================================================================
// LOG BUFFER
// =============================================================================

struct LogEntry {
  uint64_t timestampNs;
  String level;
  String message;
};

static constexpr size_t LOG_BUFFER_SIZE = 50;
static LogEntry g_logBuffer[LOG_BUFFER_SIZE];
static size_t g_logCount = 0;
static size_t g_logWriteIdx = 0;

static uint32_t g_lastPushMs = 0;
static constexpr uint32_t PUSH_INTERVAL_MS = 10000;  // Push every 10 seconds

// =============================================================================
// INTERNAL HELPERS
// =============================================================================

static uint64_t getTimestampNs() {
  /**
   * Get current timestamp in nanoseconds (Loki format).
   * Uses millis() since ESP32 doesn't have RTC by default.
   */
  return (uint64_t)millis() * 1000000ULL;
}

static void addLogEntry(const String &level, const String &message) {
  /**
   * Add a log entry to the circular buffer.
   */
  LogEntry &entry = g_logBuffer[g_logWriteIdx];
  entry.timestampNs = getTimestampNs();
  entry.level = level;
  entry.message = message;
  
  g_logWriteIdx = (g_logWriteIdx + 1) % LOG_BUFFER_SIZE;
  if (g_logCount < LOG_BUFFER_SIZE) {
    g_logCount++;
  }
}

static bool pushToLoki() {
  /**
   * Push buffered logs to Loki.
   * Returns true on success.
   */
  if (g_logCount == 0 || g_config.lokiHost.length() == 0) {
    return false;
  }
  
  if (!g_state.wifiConnected) {
    return false;
  }
  
  HTTPClient http;
  String url = g_config.lokiHost + "/loki/api/v1/push";
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  
  // Add basic auth if configured
  if (g_config.lokiUser.length() > 0) {
    String auth = g_config.lokiUser + ":" + g_config.lokiPass;
    String encoded = base64::encode(auth);
    http.addHeader("Authorization", "Basic " + encoded);
  }
  
  // Build Loki push payload
  // Format: {"streams":[{"stream":{labels},"values":[["timestamp","line"],...]}]}
  DynamicJsonDocument doc(4096);
  JsonArray streams = doc.createNestedArray("streams");
  JsonObject stream = streams.createNestedObject();
  
  // Labels
  JsonObject labels = stream.createNestedObject("stream");
  labels["job"] = "restarter";
  labels["device"] = g_state.deviceId;
  labels["hostname"] = g_state.hostname;
  
  // Values (log entries)
  JsonArray values = stream.createNestedArray("values");
  
  size_t startIdx = (g_logWriteIdx + LOG_BUFFER_SIZE - g_logCount) % LOG_BUFFER_SIZE;
  for (size_t i = 0; i < g_logCount; i++) {
    size_t idx = (startIdx + i) % LOG_BUFFER_SIZE;
    LogEntry &entry = g_logBuffer[idx];
    
    JsonArray value = values.createNestedArray();
    value.add(String(entry.timestampNs));
    value.add("[" + entry.level + "] " + entry.message);
  }
  
  String payload;
  serializeJson(doc, payload);
  
  int httpCode = http.POST(payload);
  http.end();
  
  if (httpCode == 204 || httpCode == 200) {
    g_logCount = 0;  // Clear buffer on success
    return true;
  } else {
    Serial.printf("Loki push failed: %d\n", httpCode);
    return false;
  }
}

// =============================================================================
// PUBLIC API
// =============================================================================

void Loki_log(const char *level, const char *message) {
  /**
   * Log a message to be shipped to Loki.
   * Also prints to Serial for local debugging.
   */
  Serial.printf("[%s] %s\n", level, message);
  
  if (g_config.lokiHost.length() > 0) {
    addLogEntry(String(level), String(message));
  }
}

void Loki_logf(const char *level, const char *format, ...) {
  /**
   * Printf-style logging to Loki.
   */
  char buffer[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  
  Loki_log(level, buffer);
}

void Loki_debug(const char *message) { Loki_log("DEBUG", message); }
void Loki_info(const char *message) { Loki_log("INFO", message); }
void Loki_warn(const char *message) { Loki_log("WARN", message); }
void Loki_error(const char *message) { Loki_log("ERROR", message); }

// =============================================================================
// SETUP & LOOP
// =============================================================================

void LokiHandler_setup() {
  /**
   * Initialize Loki handler.
   */
  if (g_config.lokiHost.length() > 0) {
    Serial.print("Loki logging enabled: ");
    Serial.println(g_config.lokiHost);
  } else {
    Serial.println("Loki logging: disabled (no host configured)");
  }
}

void LokiHandler_loop() {
  /**
   * Periodically push logs to Loki.
   */
  if (g_config.lokiHost.length() == 0) {
    return;
  }
  
  if (millis() - g_lastPushMs > PUSH_INTERVAL_MS) {
    g_lastPushMs = millis();
    pushToLoki();
  }
}
