/*
 * KT_MAX1726X Arduino Library
 * Copyright (C) 2026 Kacper Tora
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License.
 */

#pragma once

#include "Arduino.h"
#include "Wire.h"

#define KT_MAX1726X_ADDR 0x36

#define KT_MAX1726X_REGISTER_STATUS 0x00
#define KT_MAX1726X_REGISTER_STATUS2 0xB0
#define KT_MAX1726X_REGISTER_CONFIG 0x1D
#define KT_MAX1726X_REGISTER_CONFIG2 0xBB
#define KT_MAX1726X_REGISTER_ICHG_TERM 0x1E
#define KT_MAX1726X_REGISTER_FSTAT 0x3D
#define KT_MAX1726X_REGISTER_MODEL_CFG 0xDB
#define KT_MAX1726X_REGISTER_VCELL 0x09
#define KT_MAX1726X_REGISTER_VCELL_AVG 0x19
#define KT_MAX1726X_REGISTER_VCELL_MIN_MAX 0x1B
#define KT_MAX1726X_REGISTER_CURRENT 0x0A
#define KT_MAX1726X_REGISTER_CURRENT_AVG 0x0B
#define KT_MAX1726X_REGISTER_CURRENT_MIN_MAX 0x1C
#define KT_MAX1726X_REGISTER_TEMP 0x08
#define KT_MAX1726X_REGISTER_TEMP_AVG 0x16
#define KT_MAX1726X_REGISTER_TEMP_MIN_MAX 0x1A
#define KT_MAX1726X_REGISTER_TEMP_DIE 0x34
#define KT_MAX1726X_REGISTER_CAPACITY 0x05
#define KT_MAX1726X_REGISTER_CAPACITY_DESIGN 0x18
#define KT_MAX1726X_REGISTER_CAPACITY_FULL 0x10
#define KT_MAX1726X_REGISTER_PERCENTAGE 0x06
#define KT_MAX1726X_REGISTER_TTE 0x11
#define KT_MAX1726X_REGISTER_TTF 0x20
#define KT_MAX1726X_REGISTER_CYCLES 0x17
#define KT_MAX1726X_REGISTER_POWER 0xB1
#define KT_MAX1726X_REGISTER_POWER_AVG 0xB3
#define KT_MAX1726X_REGISTER_VEMPTY 0x3A
#define KT_MAX1726X_REGISTER_VOLTAGE_ALERT 0x01
#define KT_MAX1726X_REGISTER_TEMP_ALERT 0x02
#define KT_MAX1726X_REGISTER_SOC_ALERT 0x03
#define KT_MAX1726X_REGISTER_CURRENT_ALERT 0xB4
#define KT_MAX1726X_REGISTER_AGE 0x07
#define KT_MAX1726X_REGISTER_RESISTANCE 0x14
#define KT_MAX1726X_REGISTER_RIPPLE 0xBC
#define KT_MAX1726X_REGISTER_DEV_NAME 0x21
#define KT_MAX1726X_REGISTER_MAX_PEAK_POWER 0xD4
#define KT_MAX1726X_REGISTER_SUS_PEAK_POWER 0xD5
#define KT_MAX1726X_REGISTER_MPP_CURRENT 0xD9
#define KT_MAX1726X_REGISTER_SPP_CURRENT 0xDA
#define KT_MAX1726X_REGISTER_AT_Q_RESIDUAL 0xDC
#define KT_MAX1726X_REGISTER_AT_TTE 0xDD
#define KT_MAX1726X_REGISTER_AT_AV_SOC 0xDE
#define KT_MAX1726X_REGISTER_AT_AV_CAP 0xDF
#define KT_MAX1726X_REGISTER_PACK_RESISTANCE 0xD6
#define KT_MAX1726X_REGISTER_AT_RATE 0x04
#define KT_MAX1726X_REGISTER_SYS_RESISTANCE 0xD7
#define KT_MAX1726X_REGISTER_MIN_SYS_VOLTAGE 0xD8
#define KT_MAX1726X_REGISTER_VFSOC 0xFF
#define KT_MAX1726X_REGISTER_VFOCV 0xFB
#define KT_MAX1726X_REGISTER_QH 0x4D

#define KT_MAX1726X_POR_MASK 0x02
#define KT_MAX1726X_DNR_MASK 0x01
#define KT_MAX1726X_REFRESH_MASK 0x8000
#define KT_MAX1726X_TSEL_MASK 0x8000
#define KT_MAX1726X_ETHRM_MASK 0x10
#define KT_MAX1726X_VEMPTY_EMPTY_MASK 0xFF80
#define KT_MAX1726X_VEMPTY_RECOVERY_MASK 0x7F
#define KT_MAX1726X_BATTERY_PRESENT_MASK 0x08
#define KT_MAX1726X_BATTERY_INSERT_MASK 0x800
#define KT_MAX1726X_BATTERY_REMOVAL_MASK 0x8000
#define KT_MAX1726X_CHANGE_ALERT_MASK 0x80
#define KT_MAX1726X_STICKY_ALERT_MASK 0x7800
#define KT_MAX1726X_MIN_CURRENT_ALRT_MASK 0x04
#define KT_MAX1726X_MAX_CURRENT_ALRT_MASK 0x40
#define KT_MAX1726X_MIN_VOLTAGE_ALRT_MASK 0x100
#define KT_MAX1726X_MAX_VOLTAGE_ALRT_MASK 0x1000
#define KT_MAX1726X_MIN_TEMP_ALRT_MASK 0x200
#define KT_MAX1726X_MAX_TEMP_ALRT_MASK 0x2000
#define KT_MAX1726X_MIN_SOC_ALRT_MASK 0x400
#define KT_MAX1726X_MAX_SOC_ALRT_MASK 0x4000
#define KT_MAX1726X_ALERT_MASK 0x04
#define KT_MAX1726X_TEMP_ALERT_MASK 0x40
#define KT_MAX1726X_SOC_ALERT_MASK 0x80
#define KT_MAX1726X_ATRATEEN_MASK 0x2000
#define KT_MAX1726X_DPEN_MASK 0x1000
#define KT_MAX1726X_SNREADY_MASK 0x100

enum kt_max1726x_temp_mode {
    KT_MAX1726X_TEMP_MODE_DIE = 0,
    KT_MAX1726X_TEMP_MODE_NTC = 1
};

class KT_MAX1726X
{
    public:
    explicit KT_MAX1726X(TwoWire *wire = &Wire);

    bool begin(uint16_t designCap_mAh, float rSense = 0.010f, uint16_t iChgTerm_mA = 250, TwoWire *wire = nullptr);
    bool isConnected();

    float getVoltage();
    float getAverageVoltage();
    float getMinVoltage();
    float getMaxVoltage();
    bool resetMinMaxVoltage();
    float getCurrent();
    float getAverageCurrent();
    float getMinCurrent();
    float getMaxCurrent();
    bool resetMinMaxCurrent();
    bool setTemperatureMeasurementMode(kt_max1726x_temp_mode mode);
    float getTemp();
    float getAverageTemp();
    float getMinTemp();
    float getMaxTemp();
    float getDieTemp();
    bool resetMinMaxTemp();
    float getCapacity();
    float getDesignCapacity();
    float getFullCapacity();
    float getPercentage();
    float getTimeToEmpty();
    float getTimeToFull();
    float getCycles();
    float getPower();
    float getAveragePower();
    bool setEmptyVoltage(uint16_t mV);
    bool setRecoveryVoltage(uint16_t mV);
    bool isBatteryPresent();
    bool isBatteryInserted();
    bool isBatteryRemoved();
    bool isChangeAlert();
    bool setStickyAlert(bool enable = true);
    bool isMinCurrentAlert();
    bool isMaxCurrentAlert();
    bool isMinVoltageAlert();
    bool isMaxVoltageAlert();
    bool isMinTemperatureAlert();
    bool isMaxTemperatureAlert();
    bool isMinPercentageAlert();
    bool isMaxPercentageAlert();
    bool setMinVoltageAlert(uint16_t voltage_mV);
    bool setMaxVoltageAlert(uint16_t voltage_mV);
    bool setMinTemperatureAlert(int16_t temp_C);
    bool setMaxTemperatureAlert(int16_t temp_C);
    bool setMinPercentageAlert(uint16_t percentage);
    bool setMaxPercentageAlert(uint16_t percentage);
    bool setMinCurrentAlert(int32_t current_mA);
    bool setMaxCurrentAlert(int32_t current_mA);
    bool enableAlert(bool enable);
    bool enablePercentageAlert(bool enable);
    bool enableTemperatureAlert(bool enable);
    float getAge();
    float getResistance();
    float getRippleVoltage();
    uint16_t getDevName();
    bool getSerialNumber(uint16_t number[8]);
    float getMaxPeakPower();
    float getSusPeakPower();
    bool setPackResistance(uint16_t resistance_mOhm);
    float getPackResistance();
    bool setSysResistance(uint16_t resistance_mOhm);
    float getSysResistance();
    bool setMinSysVoltage(uint16_t voltage_mV);
    float getMinSysVoltage();
    float getMPPCurrent();
    float getSPPCurrent();
    bool setAtRate(int32_t current_mA);
    float getAtTimeToEmpty();
    float getAtAvPercentage();
    float getAtAvCapacity();
    float getAtQResidual();
    float getVFPercentage();
    float getVFVoltage();
    float getQHCapacity();

    void setDebugPort(Stream &debugPort) { _debugPort = &debugPort; }
    void disableDebug() { _debugPort = nullptr; }

    private:

    uint16_t _readRegister(uint8_t reg);
    bool _writeRegister(uint8_t reg, uint16_t value);
    bool _updateReg(uint16_t value, uint8_t reg, uint16_t mask, int offset);
    uint16_t _readReg(uint8_t reg, uint16_t mask, int offset);
    bool _isSticky();

    TwoWire* _wire;
    Stream *_debugPort; 
    float _rSense;
};