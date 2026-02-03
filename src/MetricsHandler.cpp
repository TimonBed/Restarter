/**
 * =============================================================================
 * MetricsHandler.cpp - Prometheus Metrics Endpoint
 * =============================================================================
 * 
 * Exposes device metrics in Prometheus text format at GET /metrics
 * 
 * Example scrape config for prometheus.yml:
 *   - job_name: 'restarter'
 *     static_configs:
 *       - targets: ['restarter-abc123.local:80']
 * 
 * =============================================================================
 */

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

#include "Config.h"
#include "Constants.h"

// Global objects from main.cpp
extern AsyncWebServer g_server;
extern StoredConfig g_config;
extern RuntimeState g_state;

// =============================================================================
// PROMETHEUS METRICS FORMAT
// =============================================================================

static String buildMetrics() {
  /**
   * Build Prometheus metrics in text exposition format.
   * See: https://prometheus.io/docs/instrumenting/exposition_formats/
   */
  String m;
  m.reserve(2048);
  
  String labels = String("{device=\"") + g_state.deviceId + "\",hostname=\"" + g_state.hostname + "\"}";
  
  // Device info (as label)
  m += "# HELP restarter_info Device information\n";
  m += "# TYPE restarter_info gauge\n";
  m += "restarter_info{device=\"" + g_state.deviceId + "\",hostname=\"" + g_state.hostname + 
       "\",firmware=\"" + Config::FW_VERSION + "\"} 1\n\n";
  
  // PC State (0=OFF, 1=BOOTING, 2=RUNNING, 3=RESTARTING)
  m += "# HELP restarter_pc_state PC power state (0=OFF, 1=BOOTING, 2=RUNNING, 3=RESTARTING)\n";
  m += "# TYPE restarter_pc_state gauge\n";
  m += "restarter_pc_state" + labels + " " + String(static_cast<int>(g_state.pcState)) + "\n\n";
  
  // PC power state as boolean (1=ON, 0=OFF)
  m += "# HELP restarter_pc_power PC power state (1=ON, 0=OFF)\n";
  m += "# TYPE restarter_pc_power gauge\n";
  m += "restarter_pc_power" + labels + " " + String(g_state.pcState != PCState::OFF ? 1 : 0) + "\n\n";
  
  // Relay states
  m += "# HELP restarter_power_relay Power relay active state\n";
  m += "# TYPE restarter_power_relay gauge\n";
  m += "restarter_power_relay" + labels + " " + String(g_state.powerRelayActive ? 1 : 0) + "\n\n";
  
  m += "# HELP restarter_reset_relay Reset relay active state\n";
  m += "# TYPE restarter_reset_relay gauge\n";
  m += "restarter_reset_relay" + labels + " " + String(g_state.resetRelayActive ? 1 : 0) + "\n\n";
  
  // Temperature
  m += "# HELP restarter_temperature_celsius Internal temperature in Celsius\n";
  m += "# TYPE restarter_temperature_celsius gauge\n";
  m += "restarter_temperature_celsius" + labels + " " + String(g_state.temperature, 1) + "\n\n";
  
  // HDD activity (seconds since last activity)
  m += "# HELP restarter_hdd_idle_seconds Seconds since last HDD activity\n";
  m += "# TYPE restarter_hdd_idle_seconds gauge\n";
  if (g_state.lastHddActiveMs > 0) {
    m += "restarter_hdd_idle_seconds" + labels + " " + String((millis() - g_state.lastHddActiveMs) / 1000) + "\n\n";
  } else {
    m += "restarter_hdd_idle_seconds" + labels + " -1\n\n";
  }
  
  // WiFi
  m += "# HELP restarter_wifi_connected WiFi connection state (1=connected, 0=disconnected)\n";
  m += "# TYPE restarter_wifi_connected gauge\n";
  m += "restarter_wifi_connected" + labels + " " + String(g_state.wifiConnected ? 1 : 0) + "\n\n";
  
  m += "# HELP restarter_wifi_rssi WiFi signal strength in dBm\n";
  m += "# TYPE restarter_wifi_rssi gauge\n";
  m += "restarter_wifi_rssi" + labels + " " + String(g_state.wifiConnected ? WiFi.RSSI() : 0) + "\n\n";
  
  m += "# HELP restarter_ap_mode Access Point mode active (1=AP, 0=STA)\n";
  m += "# TYPE restarter_ap_mode gauge\n";
  m += "restarter_ap_mode" + labels + " " + String(g_state.apMode ? 1 : 0) + "\n\n";
  
  // ESP32 system stats
  m += "# HELP restarter_heap_free_bytes Free heap memory in bytes\n";
  m += "# TYPE restarter_heap_free_bytes gauge\n";
  m += "restarter_heap_free_bytes" + labels + " " + String(g_state.freeHeap) + "\n\n";
  
  m += "# HELP restarter_heap_total_bytes Total heap memory in bytes\n";
  m += "# TYPE restarter_heap_total_bytes gauge\n";
  m += "restarter_heap_total_bytes" + labels + " " + String(g_state.totalHeap) + "\n\n";
  
  m += "# HELP restarter_cpu_load_percent CPU load percentage\n";
  m += "# TYPE restarter_cpu_load_percent gauge\n";
  m += "restarter_cpu_load_percent" + labels + " " + String(g_state.cpuLoad) + "\n\n";
  
  // Uptime
  m += "# HELP restarter_uptime_seconds Device uptime in seconds\n";
  m += "# TYPE restarter_uptime_seconds counter\n";
  m += "restarter_uptime_seconds" + labels + " " + String(millis() / 1000) + "\n";
  
  return m;
}

// =============================================================================
// SETUP
// =============================================================================

void MetricsHandler_setup() {
  /**
   * Register the /metrics endpoint.
   * No authentication required (Prometheus typically scrapes without auth).
   */
  g_server.on("/metrics", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain; version=0.0.4; charset=utf-8", buildMetrics());
  });
  
  Serial.println("Prometheus metrics endpoint: GET /metrics");
}

void MetricsHandler_loop() {
  // Nothing to do - metrics are generated on-demand
}
