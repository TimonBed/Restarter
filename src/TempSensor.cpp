#include "TempSensor.h"

void TempSensor::begin(uint8_t sda, uint8_t scl, uint8_t addr) {
  _addr = addr;
  _wire = &Wire;
  _wire->begin(sda, scl);
}

bool TempSensor::isConnected() {
  _wire->beginTransmission(_addr);
  return (_wire->endTransmission() == 0);
}

float TempSensor::readTemperature() {
  // TMP112 temperature register is 0x00
  _wire->beginTransmission(_addr);
  _wire->write(0x00);
  _wire->endTransmission();
  
  _wire->requestFrom(_addr, (uint8_t)2);
  if (_wire->available() < 2) {
    return NAN;
  }
  
  uint8_t msb = _wire->read();
  uint8_t lsb = _wire->read();
  
  // TMP112: 12-bit resolution, left-aligned in 16-bit register
  int16_t raw = ((msb << 8) | lsb) >> 4;
  
  // Handle negative temperatures (sign extension)
  if (raw & 0x800) {
    raw |= 0xF000;
  }
  
  // Resolution: 0.0625°C per LSB
  return raw * 0.0625f;
}
