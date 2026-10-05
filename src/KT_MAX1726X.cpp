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

  if(getDevName() == 0x4039){
    _rSense = 0.010f;
  } else if(getDevName() == 0x4031){
    _rSense = rSense;
  } else{
    _debugPort->println("Device name does not match the MAX17260 or MAX17262!");
  }

  uint16_t por = _readReg(KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_POR_MASK, 1);

  if(por == 1){

    uint32_t timeout = millis() + 1000;
    while(_readReg(KT_MAX1726X_REGISTER_FSTAT, KT_MAX1726X_DNR_MASK, 0) == 1){
        if (millis() > timeout) {
            if (_debugPort != nullptr) _debugPort->println("Timeout DNR.");
            return false;
        }
        delay(1);
    }

    _writeRegister(KT_MAX1726X_REGISTER_CAPACITY_DESIGN, (uint16_t)(designCap_mAh * rSense * 200.0f));
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

  setStickyAlert(true);

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
  return avgPower * (0.008f / _rSense);
}

bool KT_MAX1726X::setEmptyVoltage(uint16_t mV){
    return _updateReg(mV / 10.0f, KT_MAX1726X_REGISTER_VEMPTY, KT_MAX1726X_VEMPTY_EMPTY_MASK, 7);
}

bool KT_MAX1726X::setRecoveryVoltage(uint16_t mV){
    return _updateReg(mV / 40.0f, KT_MAX1726X_REGISTER_VEMPTY, KT_MAX1726X_VEMPTY_RECOVERY_MASK, 0);
}

bool KT_MAX1726X::isBatteryPresent(){
    return !_readReg(KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_BATTERY_PRESENT_MASK, 3);
}

bool KT_MAX1726X::isBatteryInserted(){
    bool insert = _readReg(KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_BATTERY_INSERT_MASK, 11);
    if(insert) {
        _updateReg(0, KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_BATTERY_INSERT_MASK, 11);
    }
    return insert;
}

bool KT_MAX1726X::isBatteryRemoved(){
    bool removal = _readReg(KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_BATTERY_REMOVAL_MASK, 15);
    if(removal){
      _updateReg(0, KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_BATTERY_REMOVAL_MASK, 15);  
    }
    return removal;
}

bool KT_MAX1726X::isChangeAlert(){
    bool alert = _readReg(KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_CHANGE_ALERT_MASK, 7);
    if(alert){
        _updateReg(0, KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_CHANGE_ALERT_MASK, 7); 
    }
    return alert;
}

bool KT_MAX1726X::setStickyAlert(bool enable){
    uint16_t val = enable ? 0b1111 : 0b0000;
    return _updateReg(val, KT_MAX1726X_REGISTER_CONFIG, KT_MAX1726X_STICKY_ALERT_MASK, 11);
}

bool KT_MAX1726X::_isSticky(){
    return _readReg(KT_MAX1726X_REGISTER_CONFIG, KT_MAX1726X_STICKY_ALERT_MASK, 11) == 0b1111;
}

bool KT_MAX1726X::isMinCurrentAlert(){
    bool alert = _readReg(KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_MIN_CURRENT_ALRT_MASK, 2);
    if(alert){
        if(_isSticky()){
            _updateReg(0, KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_MIN_CURRENT_ALRT_MASK, 2); 
        }
    }
    return alert;
}

bool KT_MAX1726X::isMaxCurrentAlert(){
    bool alert = _readReg(KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_MAX_CURRENT_ALRT_MASK, 6);
    if(alert){
        if(_isSticky()){
            _updateReg(0, KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_MAX_CURRENT_ALRT_MASK, 6); 
        }
    }
    return alert;
}

bool KT_MAX1726X::isMinVoltageAlert(){
    bool alert = _readReg(KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_MIN_VOLTAGE_ALRT_MASK, 8);
    if(alert){
        if(_isSticky()){
            _updateReg(0, KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_MIN_VOLTAGE_ALRT_MASK, 8); 
        }
    }
    return alert;
}

bool KT_MAX1726X::isMaxVoltageAlert(){
    bool alert = _readReg(KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_MAX_VOLTAGE_ALRT_MASK, 12);
    if(alert){
        if(_isSticky()){
            _updateReg(0, KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_MAX_VOLTAGE_ALRT_MASK, 12); 
        }
    }
    return alert;
}

bool KT_MAX1726X::isMinTemperatureAlert(){
    bool alert = _readReg(KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_MIN_TEMP_ALRT_MASK, 9);
    if(alert){
        if(_isSticky()){
            _updateReg(0, KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_MIN_TEMP_ALRT_MASK, 9); 
        }
    }
    return alert;
}

bool KT_MAX1726X::isMaxTemperatureAlert(){
    bool alert = _readReg(KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_MAX_TEMP_ALRT_MASK, 13);
    if(alert){
        if(_isSticky()){
            _updateReg(0, KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_MAX_TEMP_ALRT_MASK, 13); 
        }
    }
    return alert;
}

bool KT_MAX1726X::isMinPercentageAlert(){
    bool alert = _readReg(KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_MIN_SOC_ALRT_MASK, 10);
    if(alert){
        if(_isSticky()){
            _updateReg(0, KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_MIN_SOC_ALRT_MASK, 10); 
        }
    }
    return alert;
}

bool KT_MAX1726X::isMaxPercentageAlert(){
    bool alert = _readReg(KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_MAX_SOC_ALRT_MASK, 14);
    if(alert){
        if(_isSticky()){
            _updateReg(0, KT_MAX1726X_REGISTER_STATUS, KT_MAX1726X_MAX_SOC_ALRT_MASK, 14); 
        }
    }
    return alert;
}
bool KT_MAX1726X::setMinVoltageAlert(uint16_t voltage_mV){
    if(voltage_mV > 5100) voltage_mV = 5100;
    return _updateReg(voltage_mV / 20, KT_MAX1726X_REGISTER_VOLTAGE_ALERT, 0x00FF, 0);
}

bool KT_MAX1726X::setMaxVoltageAlert(uint16_t voltage_mV){
    if(voltage_mV > 5100) voltage_mV = 5100;
    return _updateReg(voltage_mV / 20, KT_MAX1726X_REGISTER_VOLTAGE_ALERT, 0xFF00, 8);
}

bool KT_MAX1726X::setMinTemperatureAlert(int16_t temp_C) {
    if (temp_C > 127)  temp_C = 127;
    if (temp_C < -128) temp_C = -128;

    return _updateReg((uint8_t)(int8_t)temp_C, KT_MAX1726X_REGISTER_TEMP_ALERT, 0x00FF, 0);
}

bool KT_MAX1726X::setMaxTemperatureAlert(int16_t temp_C) {
    if (temp_C > 127)  temp_C = 127;
    if (temp_C < -128) temp_C = -128;

    return _updateReg((uint8_t)(int8_t)temp_C, KT_MAX1726X_REGISTER_TEMP_ALERT, 0xFF00, 8);
}

bool KT_MAX1726X::setMinPercentageAlert(uint16_t percentage){
    if(percentage > 100) percentage = 100;

    return _updateReg(percentage, KT_MAX1726X_REGISTER_SOC_ALERT, 0xFF, 0);
}

bool KT_MAX1726X::setMaxPercentageAlert(uint16_t percentage){
    if(percentage > 100) percentage = 100;

    return _updateReg(percentage, KT_MAX1726X_REGISTER_SOC_ALERT, 0xFF00, 8);
}

bool KT_MAX1726X::setMinCurrentAlert(int32_t current_mA) {
    int16_t raw = (current_mA * _rSense) / 0.4f;
    if (raw > 127)  raw = 127;
    if (raw < -128) raw = -128;

    return _updateReg((uint8_t)(int8_t)raw, KT_MAX1726X_REGISTER_CURRENT_ALERT, 0x00FF, 0);
}

bool KT_MAX1726X::setMaxCurrentAlert(int32_t current_mA) {
    int16_t raw = (current_mA * _rSense) / 0.4f;
    if (raw > 127)  raw = 127;
    if (raw < -128) raw = -128;

    return _updateReg((uint8_t)(int8_t)raw, KT_MAX1726X_REGISTER_CURRENT_ALERT, 0xFF00, 8);
}

bool KT_MAX1726X::enableAlert(bool enable){
    return _updateReg(enable, KT_MAX1726X_REGISTER_CONFIG, KT_MAX1726X_ALERT_MASK, 2);
}

bool KT_MAX1726X::enablePercentageAlert(bool enable){
    return _updateReg(enable, KT_MAX1726X_REGISTER_CONFIG2, KT_MAX1726X_SOC_ALERT_MASK, 7);
}

bool KT_MAX1726X::enableTemperatureAlert(bool enable){
    return _updateReg(enable, KT_MAX1726X_REGISTER_CONFIG2, KT_MAX1726X_TEMP_ALERT_MASK, 6);
}

float KT_MAX1726X::getAge(){
    return (_readRegister(KT_MAX1726X_REGISTER_AGE) / 256.0f);
}

float KT_MAX1726X::getResistance(){
    return (_readRegister(KT_MAX1726X_REGISTER_RESISTANCE) * 1000.0f / 4096.0f);
}

float KT_MAX1726X::getRippleVoltage(){
    return (_readRegister(KT_MAX1726X_REGISTER_RIPPLE) * 0.009765625f);
}

uint16_t KT_MAX1726X::getDevName(){
    return _readRegister(KT_MAX1726X_REGISTER_DEV_NAME);
}

bool KT_MAX1726X::getSerialNumber(uint16_t number[8]){
    if (number == nullptr) return false;

    _updateReg(0, KT_MAX1726X_REGISTER_CONFIG2, KT_MAX1726X_ATRATEEN_MASK, 13);
    _updateReg(0, KT_MAX1726X_REGISTER_CONFIG2, KT_MAX1726X_DPEN_MASK, 12);

    uint32_t timeout = millis() + 15;
    while(_readReg(KT_MAX1726X_REGISTER_STATUS2, KT_MAX1726X_SNREADY_MASK, 8) == 0){
        if (millis() > timeout) {
            if (_debugPort != nullptr) _debugPort->println("Timeout SN_READY.");
            _updateReg(1, KT_MAX1726X_REGISTER_CONFIG2, KT_MAX1726X_ATRATEEN_MASK, 13);
            _updateReg(1, KT_MAX1726X_REGISTER_CONFIG2, KT_MAX1726X_DPEN_MASK, 12);
            return false;
        }
        delay(1);
    }

    uint8_t regs[8] = {KT_MAX1726X_REGISTER_MAX_PEAK_POWER, KT_MAX1726X_REGISTER_SUS_PEAK_POWER, KT_MAX1726X_REGISTER_MPP_CURRENT,
    KT_MAX1726X_REGISTER_SPP_CURRENT, KT_MAX1726X_REGISTER_AT_Q_RESIDUAL, KT_MAX1726X_REGISTER_AT_TTE, KT_MAX1726X_REGISTER_AT_AV_SOC,
    KT_MAX1726X_REGISTER_AT_AV_CAP};

    for(int i = 0; i < 8; i++){
        number[i] = _readRegister(regs[i]);
    }

    _updateReg(1, KT_MAX1726X_REGISTER_CONFIG2, KT_MAX1726X_ATRATEEN_MASK, 13);
    _updateReg(1, KT_MAX1726X_REGISTER_CONFIG2, KT_MAX1726X_DPEN_MASK, 12);

    return true;
}

float KT_MAX1726X::getMaxPeakPower(){
    return (int16_t)_readRegister(KT_MAX1726X_REGISTER_MAX_PEAK_POWER) * 0.8f;
}

float KT_MAX1726X::getSusPeakPower(){
    return (int16_t)_readRegister(KT_MAX1726X_REGISTER_SUS_PEAK_POWER) * 0.8f;
}

float KT_MAX1726X::getMPPCurrent(){
    return (int16_t)(_readRegister(KT_MAX1726X_REGISTER_MPP_CURRENT));
}

float KT_MAX1726X::getSPPCurrent(){
    return (int16_t)(_readRegister(KT_MAX1726X_REGISTER_SPP_CURRENT));
}

bool KT_MAX1726X::setPackResistance(uint16_t resistance_mOhm){
    uint16_t raw = (uint16_t)(resistance_mOhm * 4.096f);
    return _writeRegister(KT_MAX1726X_REGISTER_PACK_RESISTANCE, raw);
}

float KT_MAX1726X::getPackResistance(){
    return (_readRegister(KT_MAX1726X_REGISTER_PACK_RESISTANCE) * (1 / 4.096f));
}

bool KT_MAX1726X::setSysResistance(uint16_t resistance_mOhm){
    uint16_t raw = (uint16_t)(resistance_mOhm * 4.096f);
    return _writeRegister(KT_MAX1726X_REGISTER_SYS_RESISTANCE, raw);
}

float KT_MAX1726X::getSysResistance(){
    return (_readRegister(KT_MAX1726X_REGISTER_SYS_RESISTANCE) * (1 / 4.096f));
}

bool KT_MAX1726X::setMinSysVoltage(uint16_t voltage_mV){
    return _writeRegister(0xD8, (uint16_t)(voltage_mV / 0.078125f));
}

float KT_MAX1726X::getMinSysVoltage(){
    return _readRegister(0xD8) * 0.078125f;
}

bool KT_MAX1726X::setAtRate(int32_t current_mA){
    if (current_mA > 0) current_mA = -current_mA;

    int32_t raw = (int32_t)(current_mA * _rSense * 640.0f);
    if (raw > 32767)  raw = 32767;
    if (raw < -32768) raw = -32768;

    return _writeRegister(KT_MAX1726X_REGISTER_AT_RATE, (uint16_t)(int16_t)raw);
}

float KT_MAX1726X::getAtTimeToEmpty(){
  uint16_t atTte = _readRegister(KT_MAX1726X_REGISTER_AT_TTE);
  return atTte * 5.625f;
}

float KT_MAX1726X::getAtAvPercentage(){
  uint16_t perc = _readRegister(KT_MAX1726X_REGISTER_AT_AV_SOC);
  return perc / 256.0f;
}

float KT_MAX1726X::getAtAvCapacity(){
  uint16_t capacity = _readRegister(KT_MAX1726X_REGISTER_AT_AV_CAP);
  return capacity * (0.005f / _rSense);
}

float KT_MAX1726X::getAtQResidual(){
  uint16_t capacity = _readRegister(KT_MAX1726X_REGISTER_AT_Q_RESIDUAL);
  return capacity * (0.005f / _rSense);
}

float KT_MAX1726X::getVFPercentage(){
  uint16_t perc = _readRegister(KT_MAX1726X_REGISTER_VFSOC);
  return perc / 256.0f;
}

float KT_MAX1726X::getVFVoltage(){
  uint16_t vcell = _readRegister(KT_MAX1726X_REGISTER_VFOCV);
  return vcell * 0.000078125f;
}

float KT_MAX1726X::getQHCapacity(){
  uint16_t capacity = _readRegister(KT_MAX1726X_REGISTER_QH);
  return capacity * (0.005f / _rSense);
}