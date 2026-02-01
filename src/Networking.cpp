/**
 * =============================================================================
 * Networking.cpp - WiFi & Configuration Management
 * =============================================================================
 * 
 * This file handles all WiFi connectivity and configuration storage:
 * 
 *   - Device identity (unique hostname based on MAC address)
 *   - NVS storage (persistent config that survives firmware updates)
 *   - WiFi Station mode (connecting to your home router)
 *   - WiFi Access Point mode (for initial setup / captive portal)
 * 
 * NETWORK MODES:
 * 
 *   1. Station Mode (STA): Normal operation
 *      - Device connects to your WiFi router
 *      - You access it via the IP assigned by your router
 * 
 *   2. Access Point Mode (AP): Setup mode
 *      - Device creates its own WiFi network "Restarter-XXXXXX"
 *      - You connect to this network to configure WiFi settings
 *      - Captive portal redirects to setup wizard
 * 
 * =============================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <Preferences.h>

#include "Config.h"
#include "Constants.h"

// Global configuration and state
extern StoredConfig g_config;
extern RuntimeState g_state;

// =============================================================================
// LOCAL OBJECTS
// =============================================================================

static DNSServer g_dnsServer;  // DNS server for captive portal (redirects all domains)
static Preferences g_prefs;     // ESP32 NVS (Non-Volatile Storage) for config

// =============================================================================
// DEVICE IDENTITY
// =============================================================================

static String formatDeviceId(uint64_t mac) {
  /**
   * Convert the ESP32's MAC address to a short hex string.
   * This provides a unique, stable device ID.
   * 
   * Example: MAC 12:34:56:78:9A:BC → "123456789abc"
   */
  char buf[13] = {0};
  snprintf(buf, sizeof(buf), "%02x%02x%02x%02x%02x%02x",
           (uint8_t)(mac >> 40), (uint8_t)(mac >> 32),
           (uint8_t)(mac >> 24), (uint8_t)(mac >> 16),
           (uint8_t)(mac >> 8), (uint8_t)(mac));
  return String(buf);
}

static void loadIdentity() {
  /**
   * Generate device identity from the ESP32's MAC address.
   * 
   * This creates:
   *   - deviceId: Full hex MAC (e.g., "a1b2c3d4e5f6")
   *   - hostname: "restarter-a1b2c3d4e5f6"
   * 
   * No passwords or secrets are used - just the hardware ID.
   */
  uint64_t mac = ESP.getEfuseMac();
  g_state.deviceId = formatDeviceId(mac);
  g_state.hostname = String(Config::HOSTNAME_PREFIX) + g_state.deviceId;
  
  Serial.print("Device ID: ");
  Serial.println(g_state.deviceId);
  Serial.print("Hostname: ");
  Serial.println(g_state.hostname);
}

// =============================================================================
// CONFIGURATION STORAGE (NVS)
// =============================================================================
// NVS (Non-Volatile Storage) is a key-value store in flash memory.
// Unlike LittleFS, NVS data survives firmware and filesystem uploads.
// Only a factory reset or flash erase clears NVS data.

bool Networking_hasConfig() {
  /**
   * Check if WiFi has been configured.
   * We consider WiFi configured if an SSID is saved.
   */
  return g_config.wifiSsid.length() > 0;
}

bool Networking_loadConfig() {
  /**
   * Load all configuration from NVS.
   * Called once during setup().
   * 
   * @return true if WiFi config exists, false if unconfigured
   */
  Serial.println("Loading config from NVS...");
  
  g_prefs.begin("restarter", true);  // Open namespace in read-only mode
  
  // WiFi settings
  g_config.wifiSsid = g_prefs.getString("wifiSsid", "");
  g_config.wifiPass = g_prefs.getString("wifiPass", "");
  
  // MQTT settings
  g_config.mqttHost = g_prefs.getString("mqttHost", "");
  g_config.mqttPort = g_prefs.getUShort("mqttPort", 1883);
  g_config.mqttUser = g_prefs.getString("mqttUser", "");
  g_config.mqttPass = g_prefs.getString("mqttPass", "");
  
  // Timing settings
  g_config.powerPulseMs = g_prefs.getULong("powerPulseMs", 500);
  g_config.resetPulseMs = g_prefs.getULong("resetPulseMs", 500);
  g_config.bootGraceMs = g_prefs.getULong("bootGraceMs", 60000);
  
  g_prefs.end();
  
  if (Networking_hasConfig()) {
    Serial.print("Loaded WiFi SSID: ");
    Serial.println(g_config.wifiSsid);
  } else {
    Serial.println("No WiFi config found - will start AP mode");
  }
  
  return Networking_hasConfig();
}

bool Networking_saveConfig(const StoredConfig &cfg) {
  /**
   * Save configuration to NVS.
   * Called when user submits the setup form.
   * 
   * @param cfg  The new configuration to save
   * @return true on success
   */
  Serial.println("Saving config to NVS...");
  
  g_prefs.begin("restarter", false);  // Open namespace in read-write mode
  
  // WiFi settings
  g_prefs.putString("wifiSsid", cfg.wifiSsid);
  g_prefs.putString("wifiPass", cfg.wifiPass);
  
  // MQTT settings
  g_prefs.putString("mqttHost", cfg.mqttHost);
  g_prefs.putUShort("mqttPort", cfg.mqttPort);
  g_prefs.putString("mqttUser", cfg.mqttUser);
  g_prefs.putString("mqttPass", cfg.mqttPass);
  
  // Timing settings
  g_prefs.putULong("powerPulseMs", cfg.powerPulseMs);
  g_prefs.putULong("resetPulseMs", cfg.resetPulseMs);
  g_prefs.putULong("bootGraceMs", cfg.bootGraceMs);
  
  g_prefs.end();
  
  // Update global config
  g_config = cfg;
  
  Serial.println("Config saved successfully");
  return true;
}

bool Networking_clearConfig() {
  /**
   * Erase ALL configuration from NVS.
   * This is the "factory reset" function.
   * 
   * After calling this, the device will boot into AP mode
   * for fresh setup.
   */
  Serial.println("!!! CLEARING ALL CONFIG (FACTORY RESET) !!!");
  
  g_prefs.begin("restarter", false);  // Open in read-write mode
  g_prefs.clear();                     // Erase all keys in namespace
  g_prefs.end();
  
  // Reset in-memory config to defaults
  g_config = StoredConfig();
  
  Serial.println("Config cleared - device will start in AP mode on next boot");
  return true;
}

// =============================================================================
// WIFI CONNECTION
// =============================================================================

static bool connectSta() {
  /**
   * Connect to a WiFi router using saved credentials.
   * 
   * This function blocks for up to WIFI_CONNECT_TIMEOUT_MS (15 seconds)
   * while attempting to connect.
   * 
   * @return true if connected, false if failed
   */
  if (!Networking_hasConfig()) {
    return false;
  }
  
  Serial.print("Connecting to WiFi: ");
  Serial.println(g_config.wifiSsid);
  
  // Set WiFi mode to Station (client)
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(g_state.hostname.c_str());
  WiFi.begin(g_config.wifiSsid.c_str(), g_config.wifiPass.c_str());

  // Wait for connection (with timeout)
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && 
         millis() - start < Config::WIFI_CONNECT_TIMEOUT_MS) {
    delay(50);
    Serial.print(".");
  }
  Serial.println();
  
  g_state.wifiConnected = (WiFi.status() == WL_CONNECTED);
  
  if (g_state.wifiConnected) {
    Serial.print("Connected! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Connection failed!");
  }
  
  return g_state.wifiConnected;
}

static void startAp() {
  /**
   * Start Access Point mode for initial setup.
   * 
   * Creates a WiFi network named "Restarter-XXXXXX" (last 6 chars of device ID).
   * Also starts a DNS server that redirects ALL domains to this device,
   * which triggers the captive portal on most phones/computers.
   */
  WiFi.mode(WIFI_AP);
  
  // Create AP SSID using last 6 characters of device ID
  String ssid = String(Config::AP_SSID_PREFIX) + g_state.deviceId.substring(6);
  WiFi.softAP(ssid.c_str());
  
  // Start DNS server for captive portal (redirect all domains to 192.168.4.1)
  g_dnsServer.start(53, "*", WiFi.softAPIP());
  
  g_state.apMode = true;
  g_state.wifiConnected = false;
  
  Serial.println("=== ACCESS POINT MODE ===");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("Connect to this network to configure the device");
}

// =============================================================================
// INITIALIZATION
// =============================================================================

void Networking_setup() {
  /**
   * Initialize networking subsystem.
   * Called once during setup().
   * 
   * This function:
   *   1. Generates device identity from MAC
   *   2. Initializes the status LED
   *   3. Mounts LittleFS for web files
   *   4. Loads config from NVS
   *   5. Connects to WiFi or starts AP mode
   */
  
  // Generate unique device ID and hostname
  loadIdentity();
  
  // Initialize status LED (used for WiFi/reset feedback)
  pinMode(Config::PIN_WIFI_ERROR_LED, OUTPUT);
  digitalWrite(Config::PIN_WIFI_ERROR_LED, LOW);
  
  // Mount LittleFS filesystem (contains web UI files)
  if (!LittleFS.begin(true)) {
    Serial.println("WARNING: LittleFS mount failed!");
  }
  
  // Load saved configuration
  Networking_loadConfig();

  // Decide: connect to WiFi or start setup AP?
  if (!Networking_hasConfig()) {
    // No WiFi configured - start AP for setup
    startAp();
  } else {
    // WiFi configured - try to connect
    g_state.apMode = false;
    if (!connectSta()) {
      // Connection failed - turn on error LED
      digitalWrite(Config::PIN_WIFI_ERROR_LED, HIGH);
    }
  }
}

// =============================================================================
// MAIN LOOP
// =============================================================================

void Networking_loop() {
  /**
   * Handle ongoing network tasks.
   * Called continuously in loop().
   * 
   * In AP mode:
   *   - Process DNS requests (for captive portal)
   *   - Blink LED slowly (1 second on/off)
   *   - Restart after timeout if config exists
   * 
   * In STA mode:
   *   - Monitor connection status
   *   - Reconnect if disconnected
   *   - Update LED status
   */
  static uint32_t lastAttemptMs = 0;
  static uint32_t apStartMs = millis();

  // -------------------------------------------------------------------------
  // Access Point Mode
  // -------------------------------------------------------------------------
  if (g_state.apMode) {
    // Process captive portal DNS requests
    g_dnsServer.processNextRequest();
    
    // Blink LED slowly to indicate AP mode (1s on, 1s off)
    static uint32_t lastBlinkMs = 0;
    static bool ledState = false;
    if (millis() - lastBlinkMs > 1000) {
      lastBlinkMs = millis();
      ledState = !ledState;
      digitalWrite(Config::PIN_WIFI_ERROR_LED, ledState ? HIGH : LOW);
    }
    
    // If we have a saved config but are in AP mode (e.g., after failed connect),
    // restart after the idle timeout to try connecting again
    if (millis() - apStartMs > Config::AP_IDLE_TIMEOUT_MS && Networking_hasConfig()) {
      Serial.println("AP idle timeout - restarting to retry WiFi connection");
      ESP.restart();
    }
    return;
  }

  // -------------------------------------------------------------------------
  // Station Mode
  // -------------------------------------------------------------------------
  
  // Check if we're connected
  if (WiFi.status() == WL_CONNECTED) {
    g_state.wifiConnected = true;
    digitalWrite(Config::PIN_WIFI_ERROR_LED, LOW);  // LED off = connected
    return;
  }
  
  // Not connected - update state and LED
  g_state.wifiConnected = false;
  digitalWrite(Config::PIN_WIFI_ERROR_LED, HIGH);  // LED on = error
  
  // Try to reconnect periodically
  if (millis() - lastAttemptMs > Config::WIFI_CONNECT_TIMEOUT_MS) {
    lastAttemptMs = millis();
    Serial.println("WiFi disconnected - attempting reconnection...");
    connectSta();
  }
}
