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
  _rSense = 0.010f;
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

bool KT_MAX1726X::isConnected() {
  _wire->beginTransmission(KT_MAX1726X_ADDR);
  return (_wire->endTransmission() == 0);
}

bool KT_MAX1726X::begin(uint16_t designCap_mAh, float rSense, uint16_t iChgTerm_mA, TwoWire *wire){

  if (wire != nullptr) _wire = wire;

  if(!isConnected()){

    if (_debugPort != nullptr) _debugPort->println("MAX1726X is not connected!");
    return false;
  }

  _rSense = rSense;

  uint16_t por = _readReg(KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_POR_MASK, 1);

  if(por == 1){

    uint32_t timeout = millis() + 250;
    while(_readReg(KT_MAX1726X_REGISTER_FSTAT, KT_MAX1726X_DNR_MASK, 0) == 1){
        if (millis() > timeout) {
            if (_debugPort != nullptr) _debugPort->println("Timeout DNR.");
            return false;
        }
        delay(1);
    }

    _writeRegister(KT_MAX1726X_REGISTER_DESIGN_CAP, (uint16_t)(designCap_mAh * rSense * 200.0f));
    _writeRegister(KT_MAX1726X_REGISTER_ICHG_TERM, (uint16_t)(iChgTerm_mA * rSense * 640));

    _updateReg(1, KT_MAX1726X_REGISTER_MODEL_CFG, KT_MAX1726X_REFRESH_MASK, 15);

    timeout = millis() + 250;
    while(_readReg(KT_MAX1726X_REGISTER_MODEL_CFG, KT_MAX1726X_REFRESH_MASK, 15) == 1){
        if (millis() > timeout) {
            if (_debugPort != nullptr) _debugPort->println("Timeout refresh");
            return false;
        }
        delay(1);
    }

    _updateReg(0, KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_POR_MASK, 1);
    
  }

  return true;
}

float KT_MAX1726X::getVoltage(){
    uint16_t vcell = _readRegister(KT_MAX1726X_REGISTER_VCELL);
    return vcell * 0.000078125f;
}

float KT_MAX1726X::getAverageVoltage(){
    uint16_t avgVcell = _readRegister(KT_MAX1726X_REGISTER_VCELL_AVG);
    return avgVcell * 0.000078125f;
}

float KT_MAX1726X::getMinVoltage(){
    uint16_t minVcell = _readReg(KT_MAX1726X_REGISTER_VCELL_MIN_MAX, 0x00FF, 0);
    return minVcell * 0.02f;
}

float KT_MAX1726X::getMaxVoltage(){
    uint16_t maxVcell = _readReg(KT_MAX1726X_REGISTER_VCELL_MIN_MAX, 0xFF00, 8);
    return maxVcell * 0.02f;
}

bool KT_MAX1726X::resetMinMaxVoltage() {
    return _writeRegister(KT_MAX1726X_REGISTER_VCELL_MIN_MAX, 0x00FF);
}

float KT_MAX1726X::getCurrent() {
  int16_t current = (int16_t)_readRegister(KT_MAX1726X_REGISTER_CURRENT);
  return (current * 0.0015625f) / _rSense;
}

float KT_MAX1726X::getAverageCurrent(){
  int16_t avgCurrent = (int16_t)_readRegister(KT_MAX1726X_REGISTER_CURRENT_AVG);
  return (avgCurrent * 0.0015625f) / _rSense;
}

float KT_MAX1726X::getMinCurrent(){
    int8_t minCurrent = (int8_t)_readReg(KT_MAX1726X_REGISTER_CURRENT_MIN_MAX, 0x00FF, 0);
    return (minCurrent * 0.4f) / _rSense;
}

float KT_MAX1726X::getMaxCurrent(){
    int8_t maxCurrent = (int8_t)_readReg(KT_MAX1726X_REGISTER_CURRENT_MIN_MAX, 0xFF00, 8);
    return (maxCurrent * 0.4f) / _rSense;
}

bool KT_MAX1726X::resetMinMaxCurrent() {
    return _writeRegister(KT_MAX1726X_REGISTER_CURRENT_MIN_MAX, 0x807F);
}

bool KT_MAX1726X::setTemperatureMeasurementMode(kt_max1726x_temp_mode mode){
    _updateReg(mode, KT_MAX1726X_REGISTER_CONFIG, KT_MAX1726X_ETHRM_MASK, 4);
    return _updateReg(mode, KT_MAX1726X_REGISTER_CONFIG, KT_MAX1726X_TSEL_MASK, 15);
}

float KT_MAX1726X::getTemp(){
  int16_t temp = (int16_t)_readRegister(KT_MAX1726X_REGISTER_TEMP);
  return temp / 256.0f;
}

float KT_MAX1726X::getAverageTemp(){
  int16_t avgTemp = (int16_t)_readRegister(KT_MAX1726X_REGISTER_TEMP_AVG);
  return avgTemp / 256.0f;
}

float KT_MAX1726X::getMinTemp(){
    return (int8_t)_readReg(KT_MAX1726X_REGISTER_TEMP_MIN_MAX, 0x00FF, 0);
}

float KT_MAX1726X::getMaxTemp(){
    return (int8_t)_readReg(KT_MAX1726X_REGISTER_TEMP_MIN_MAX, 0xFF00, 8);
}

bool KT_MAX1726X::resetMinMaxTemp() {
    return _writeRegister(KT_MAX1726X_REGISTER_TEMP_MIN_MAX, 0x807F);
}

float KT_MAX1726X::getDieTemp(){
  int16_t dieTemp = (int16_t)_readRegister(KT_MAX1726X_REGISTER_TEMP_DIE);
  return dieTemp / 256.0f;
}

float KT_MAX1726X::getCapacity(){
  uint16_t capacity = _readRegister(KT_MAX1726X_REGISTER_CAPACITY);
  return capacity * (0.005f / _rSense);
}

float KT_MAX1726X::getDesignCapacity(){
  uint16_t designCapacity = _readRegister(KT_MAX1726X_REGISTER_CAPACITY_DESIGN);
  return designCapacity * (0.005f / _rSense);
}

float KT_MAX1726X::getFullCapacity(){
  uint16_t fullCapacity = _readRegister(KT_MAX1726X_REGISTER_CAPACITY_FULL);
  return fullCapacity * (0.005f / _rSense);
}

float KT_MAX1726X::getPercentage(){
  uint16_t percentage = _readRegister(KT_MAX1726X_REGISTER_PERCENTAGE);
  return percentage / 256.0f;
}

float KT_MAX1726X::getTimeToEmpty(){
  uint16_t tte = _readRegister(KT_MAX1726X_REGISTER_TTE);
  return tte * 5.625f;
}

float KT_MAX1726X::getTimeToFull(){
  uint16_t ttf = _readRegister(KT_MAX1726X_REGISTER_TTF);
  return ttf * 5.625f;
}

float KT_MAX1726X::getCycles(){
  uint16_t cycles = _readRegister(KT_MAX1726X_REGISTER_CYCLES);
  return cycles / 100.0f;
}

float KT_MAX1726X::getPower(){
  int16_t power = (int16_t)_readRegister(KT_MAX1726X_REGISTER_POWER);
  return power * (0.008f / _rSense);
}

float KT_MAX1726X::getAveragePower(){
  int16_t avgPower = (int16_t)_readRegister(KT_MAX1726X_REGISTER_POWER_AVG);
  return avgPower * (0.00f / _rSense);
}

bool KT_MAX1726X::setEmptyVoltage(uint16_t mV){
    return _updateReg(mV / 10.0f, KT_MAX1726X_REGISTER_VEMPTY, KT_MAX1726X_VEMPTY_EMPTY_MASK, 7);
}

bool KT_MAX1726X::setRecoveryVoltage(uint16_t mV){
    return _updateReg(mV / 40.0f, KT_MAX1726X_REGISTER_VEMPTY, KT_MAX1726X_VEMPTY_RECOVERY_MASK, 0);
}