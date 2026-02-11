/**
 * Config.h - Hardware Configuration & Default Settings
 * 
 * Pin assignments and timing values for PC Restarter.
 */

#pragma once

#include <Arduino.h>

namespace Config {

// Firmware version string
constexpr char FW_VERSION[] = "0.2.0";
constexpr char OTA_RELEASES_API[] = "https://api.github.com/repos/TimonBed/Restarter/releases/latest";
constexpr uint32_t OTA_CHECK_TIMEOUT_MS = 15000;
constexpr uint32_t OTA_DOWNLOAD_TIMEOUT_MS = 30000;

// GPIO pin assignments (edit for your wiring)
constexpr uint8_t PIN_PWR_LED = 4;        // PC power LED
constexpr uint8_t PIN_HDD_LED = 5;        // PC HDD activity LED
constexpr uint8_t PIN_FACTORY_RESET = 9;  // Factory reset button

constexpr uint8_t PIN_RELAY_POWER = 6;    // Power button relay
constexpr uint8_t PIN_RELAY_RESET = 7;    // Reset button relay
constexpr uint8_t PIN_WIFI_ERROR_LED = 10; // Status LED

// Signal polarity (set to match your hardware)
constexpr bool PWR_LED_ACTIVE_HIGH = false;
constexpr bool HDD_LED_ACTIVE_HIGH = false;
constexpr bool POWER_RELAY_ACTIVE_HIGH = true;
constexpr bool RESET_RELAY_ACTIVE_HIGH = true;

// Timing configuration (milliseconds)
// Change via web interface if supported
constexpr uint32_t POWER_PULSE_MS = 500;
constexpr uint32_t RESET_PULSE_MS = 500;
constexpr uint32_t FORCE_SHUTDOWN_PULSE_MS = 11000;

constexpr uint32_t BOOT_GRACE_MS = 60000;
constexpr uint32_t FACTORY_RESET_HOLD_MS = 5000;

constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000;
constexpr uint32_t AP_IDLE_TIMEOUT_MS = 300000;
constexpr uint32_t MQTT_RECONNECT_MS = 5000;

constexpr uint32_t STATUS_BROADCAST_MS = 1000;

// Storage & identity
constexpr char CONFIG_PATH[] = "/config.json";
constexpr char HOSTNAME_PREFIX[] = "restarter-";
constexpr char AP_SSID_PREFIX[] = "Restarter-";

// Development: fixed passwords (enable via build_flags -DRESTARTER_DEV_AP_PASSWORD)
#ifdef RESTARTER_DEV_AP_PASSWORD
constexpr char AP_PASSWORD_DEV[] = "Test1234";   // WPA2 requires 8–63 chars
constexpr char ADMIN_PASSWORD_DEV[] = "Klaus1234";  // Web UI admin
#endif

} // namespace Config
