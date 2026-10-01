/*
 * KT_MAX1726X Arduino Library
 * Copyright (C) 2026 Kacper Tora
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License.
 */

#include "KT_MAX1726X.h"

KT_MAX1726X::KT_MAX1726X(TwoWire *wire){
  _wire = wire;
  _debugPort = nullptr;
}

uint16_t KT_MAX1726X::_readRegister(uint8_t reg) {
  _wire -> beginTransmission(KT_MAX1726X_ADDR);
  _wire -> write(reg);
  _wire -> endTransmission(false);

  _wire -> requestFrom((uint8_t)KT_MAX1726X_ADDR, (uint8_t)2);

  if (_wire -> available() >= 2) {
    uint8_t lsb = _wire -> read();
    uint8_t msb = _wire -> read();
    return ((uint16_t)msb << 8) | lsb;
  }
  return 0x00;
}

bool KT_MAX1726X::_writeRegister(uint8_t reg, uint16_t value){

  _wire -> beginTransmission(KT_MAX1726X_ADDR);
  _wire -> write(reg);

  uint8_t lsb = value & 0xFF;
  uint8_t msb = value >> 8;

  _wire -> write(lsb);
  _wire -> write(msb);

  return (_wire -> endTransmission() == 0);
}

bool KT_MAX1726X::_updateReg(uint16_t value, uint8_t reg, uint16_t mask, int offset){
  uint16_t regVal = _readRegister(reg);
  regVal &= ~mask;
  regVal |= ((value << offset) & mask);
  return _writeRegister(reg, regVal);
}

uint16_t KT_MAX1726X::_readReg(uint8_t reg, uint16_t mask, int offset){
  uint16_t regVal = _readRegister(reg);
  regVal &= mask;
  return regVal >> offset;
}