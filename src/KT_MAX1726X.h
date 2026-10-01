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

class KT_MAX1726X
{
    public:
    explicit KT_MAX1726X(TwoWire *wire = &Wire);

    bool begin(TwoWire *wire = nullptr);
    bool isConnected();

    void setDebugPort(Stream &debugPort) { _debugPort = &debugPort; }
    void disableDebug() { _debugPort = nullptr; }

    private:

    uint16_t _readRegister(uint8_t reg);
    bool _writeRegister(uint8_t reg, uint16_t value);
    bool _updateReg(uint16_t value, uint8_t reg, uint16_t mask, int offset);
    uint16_t _readReg(uint8_t reg, uint16_t mask, int offset);

    TwoWire* _wire;
    Stream *_debugPort; 
};