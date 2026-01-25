#pragma once

#include <Arduino.h>

namespace Config {
constexpr char FW_VERSION[] = "0.1.0";
// GPIO defaults for ESP32 (adjust in Config.h)
constexpr uint8_t PIN_PWR_LED = 4;
constexpr uint8_t PIN_RELAY_POWER = 6;
constexpr uint8_t PIN_RELAY_RESET = 7;

// Signal polarity
constexpr bool PWR_LED_ACTIVE_HIGH = true;
constexpr bool POWER_RELAY_ACTIVE_HIGH = false; // active low default
constexpr bool RESET_RELAY_ACTIVE_HIGH = false; // active low default

// Timing
constexpr uint32_t BOOT_GRACE_MS = 60000;
constexpr uint32_t POWER_PULSE_MS = 500;
constexpr uint32_t RESET_PULSE_MS = 250;
constexpr uint32_t STATUS_BROADCAST_MS = 1000;
constexpr uint32_t MQTT_RECONNECT_MS = 5000;
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000;
constexpr uint32_t AP_IDLE_TIMEOUT_MS = 300000;

// Storage / identity
constexpr char CONFIG_PATH[] = "/config.json";
constexpr char HOSTNAME_PREFIX[] = "restarter-";
constexpr char AP_SSID_PREFIX[] = "Restarter-";
} // namespace Config
