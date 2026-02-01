/**
 * =============================================================================
 * TempSensor.h - TMP112 Temperature Sensor Driver
 * =============================================================================
 * 
 * This class reads temperature from a TMP112 I2C temperature sensor.
 * 
 * TMP112 SPECIFICATIONS:
 *   - Temperature range: -40°C to +125°C
 *   - Resolution: 0.0625°C (12-bit)
 *   - Accuracy: ±0.5°C (typical)
 *   - I2C address: 0x48 (default), can be 0x49, 0x4A, or 0x4B
 * 
 * WIRING:
 *   TMP112        ESP32
 *   ──────        ─────
 *   VCC    ───→   3.3V
 *   GND    ───→   GND
 *   SDA    ───→   GPIO 0 (default)
 *   SCL    ───→   GPIO 1 (default)
 * 
 * The sensor can be used to monitor ambient temperature near the PC,
 * which can be displayed on the web interface.
 * 
 * =============================================================================
 */

#pragma once

#include <Arduino.h>
#include <Wire.h>

class TempSensor {
public:
  /**
   * Initialize the I2C bus and sensor.
   * 
   * @param sda   I2C data pin (default: GPIO 0)
   * @param scl   I2C clock pin (default: GPIO 1)
   * @param addr  I2C address of the TMP112 (default: 0x48)
   */
  void begin(uint8_t sda = 0, uint8_t scl = 1, uint8_t addr = 0x48);
  
  /**
   * Read the current temperature in Celsius.
   * 
   * @return Temperature in °C, or NAN if sensor not connected
   */
  float readTemperature();
  
  /**
   * Check if the sensor is connected and responding.
   * 
   * @return true if sensor responds to I2C, false otherwise
   */
  bool isConnected();

private:
  uint8_t _addr = 0x48;      // I2C address
  TwoWire* _wire = &Wire;    // I2C bus instance
};
