#pragma once

#include <Arduino.h>
#include <Wire.h>

class TempSensor {
public:
  // Initialize I2C and sensor
  void begin(uint8_t sda = 0, uint8_t scl = 1, uint8_t addr = 0x48);
  
  // Read temperature in Celsius
  float readTemperature();
  
  // Check if sensor is connected
  bool isConnected();

private:
  uint8_t _addr = 0x48;
  TwoWire* _wire = &Wire;
};
