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

#define KT_MAX1726X_ADDR 0x6C

#define KT_MAX1726X_REGISTER_STATUS 0x00
#define KT_MAX1726X_REGISTER_DESIGN_CAP 0x18
#define KT_MAX1726X_REGISTER_ICHG_TERM 0x1E
#define KT_MAX1726X_REGISTER_FSTAT 0x3D
#define KT_MAX1726X_REGISTER_MODEL_CFG 0xDB
#define KT_MAX1726X_REGISTER_VCELL 0x09
#define KT_MAX1726X_REGISTER_VCELL_AVG 0x19
#define KT_MAX1726X_REGISTER_VCELL_MIN_MAX 0x1B

#define KT_MAX1726X_POR_MASK 0x02
#define KT_MAX1726X_DNR_MASK 0x01
#define KT_MAX1726X_REFRESH_MASK 0x8000

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

    void setDebugPort(Stream &debugPort) { _debugPort = &debugPort; }
    void disableDebug() { _debugPort = nullptr; }

    private:

    uint16_t _readRegister(uint8_t reg);
    bool _writeRegister(uint8_t reg, uint16_t value);
    bool _updateReg(uint16_t value, uint8_t reg, uint16_t mask, int offset);
    uint16_t _readReg(uint8_t reg, uint16_t mask, int offset);

    TwoWire* _wire;
    Stream *_debugPort; 
    float _rSense;
};