/**
 * =============================================================================
 * TempSensor.cpp - TMP112 Temperature Sensor Implementation
 * =============================================================================
 * 
 * This file implements the driver for the TMP112 I2C temperature sensor.
 * 
 * TMP112 REGISTER MAP:
 *   - 0x00: Temperature Register (read-only)
 *   - 0x01: Configuration Register
 *   - 0x02: Low Temperature Threshold
 *   - 0x03: High Temperature Threshold
 * 
 * TEMPERATURE FORMAT:
 *   - 12-bit resolution, left-aligned in 16-bit register
 *   - Resolution: 0.0625°C per LSB
 *   - Two's complement for negative temperatures
 * 
 * =============================================================================
 */

#include "TempSensor.h"

// =============================================================================
// INITIALIZATION
// =============================================================================

void TempSensor::begin(uint8_t sda, uint8_t scl, uint8_t addr) {
  /**
   * Initialize the I2C bus and store the sensor address.
   * 
   * @param sda   I2C data pin (GPIO 0 on ESP32-C3)
   * @param scl   I2C clock pin (GPIO 1 on ESP32-C3)
   * @param addr  I2C address of TMP112 (0x48-0x4B depending on ADD0 pin)
   */
  _addr = addr;
  _wire = &Wire;
  _wire->begin(sda, scl);
}

// =============================================================================
// CONNECTION CHECK
// =============================================================================

bool TempSensor::isConnected() {
  /**
   * Check if the sensor responds to its I2C address.
   * 
   * This sends an I2C start condition and address, then checks
   * if the device acknowledges. Useful for detecting if the
   * sensor is properly connected.
   * 
   * @return true if sensor responds, false if not found
   */
  _wire->beginTransmission(_addr);
  return (_wire->endTransmission() == 0);
}

// =============================================================================
// TEMPERATURE READING
// =============================================================================

float TempSensor::readTemperature() {
  /**
   * Read the current temperature from the sensor.
   * 
   * PROTOCOL:
   *   1. Write the register address (0x00 for temperature)
   *   2. Read 2 bytes (MSB first, then LSB)
   *   3. Convert the 12-bit value to Celsius
   * 
   * DATA FORMAT:
   *   The temperature is stored as a 12-bit value, left-aligned
   *   in a 16-bit register. We need to shift right by 4 bits.
   * 
   *   MSB                 LSB
   *   [D11 D10 D9 D8 D7 D6 D5 D4] [D3 D2 D1 D0 X X X X]
   *   
   *   Combined and shifted: (MSB << 8 | LSB) >> 4
   * 
   * @return Temperature in Celsius, or NAN if read fails
   */
  
  // Step 1: Select the temperature register (0x00)
  _wire->beginTransmission(_addr);
  _wire->write(0x00);  // Temperature register address
  _wire->endTransmission();
  
  // Step 2: Request 2 bytes of data
  _wire->requestFrom(_addr, (uint8_t)2);
  if (_wire->available() < 2) {
    // Sensor didn't respond with enough data
    return NAN;
  }
  
  // Step 3: Read the raw data
  uint8_t msb = _wire->read();  // Most significant byte
  uint8_t lsb = _wire->read();  // Least significant byte
  
  // Step 4: Combine bytes and shift to get 12-bit value
  // The data is left-aligned, so we shift right by 4 bits
  int16_t raw = ((msb << 8) | lsb) >> 4;
  
  // Step 5: Handle negative temperatures (sign extension)
  // If bit 11 is set, the temperature is negative (two's complement)
  if (raw & 0x800) {
    raw |= 0xF000;  // Extend the sign bit to fill 16-bit int
  }
  
  // Step 6: Convert to Celsius
  // Each LSB represents 0.0625°C (1/16 degree)
  return raw * 0.0625f;
}
