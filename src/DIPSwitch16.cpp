/**
 * DIPSwitch16.cpp
 * Implementation for the DIPSwitch16 Arduino library.
 *
 * See DIPSwitch16.h for full documentation and wiring details.
 */

#include "DIPSwitch16.h"

// ── Constructor ───────────────────────────────────────────────────────────────

DIPSwitch16::DIPSwitch16(uint8_t address, TwoWire &wire)
    : _wire(wire), _address(address), _initialised(false)
{
    // Clamp to the valid MCP23017 address range at construction time.
    // begin() will validate and return an error code if still out of range.
    if (_address < 0x20 || _address > 0x27) {
        _address = MCP23017_DEFAULT_ADDR;
    }
}

// ── Public: begin ─────────────────────────────────────────────────────────────

uint8_t DIPSwitch16::begin()
{
    // Validate address range
    if (_address < 0x20 || _address > 0x27) {
        return DIPSWITCH16_ERR_ADDR;
    }

    // Start Wire bus if it hasn't been started yet.
    // Calling begin() more than once on Wire is safe on most cores.
    _wire.begin();

    // Verify the chip is actually there before touching registers
    if (!isConnected()) {
        return DIPSWITCH16_ERR_I2C;
    }

    // ── Configure all 16 GPIO pins as INPUTS ─────────────────────────────────
    // IODIRA / IODIRB: 1 = input (this is also the power-on default,
    // but we set it explicitly so re-initialisation works correctly).
    if (!_writeReg(MCP23017_IODIRA, 0xFF)) return DIPSWITCH16_ERR_I2C;
    if (!_writeReg(MCP23017_IODIRB, 0xFF)) return DIPSWITCH16_ERR_I2C;

    // ── Enable internal 100 kΩ pull-up resistors on all pins ─────────────────
    // GPPUA / GPPUB: 1 = pull-up enabled.
    // With pull-ups active:
    //   Switch OPEN  → pin floats HIGH → we invert → reports 0 (OFF)
    //   Switch CLOSED→ pin pulled LOW  → we invert → reports 1 (ON)
    if (!_writeReg(MCP23017_GPPUA, 0xFF)) return DIPSWITCH16_ERR_I2C;
    if (!_writeReg(MCP23017_GPPUB, 0xFF)) return DIPSWITCH16_ERR_I2C;

    // ── Disable input polarity inversion (we do it in software) ──────────────
    // IPOLA / IPOLB: 0 = raw pin logic (we'll invert manually in read())
    if (!_writeReg(MCP23017_IPOLA, 0x00)) return DIPSWITCH16_ERR_I2C;
    if (!_writeReg(MCP23017_IPOLB, 0x00)) return DIPSWITCH16_ERR_I2C;

    _initialised = true;
    return DIPSWITCH16_OK;
}

// ── Public: read (16-bit) ─────────────────────────────────────────────────────

uint16_t DIPSwitch16::read()
{
    bool ok = true;

    // Read Port A (DIP Switch 1) and Port B (DIP Switch 2)
    uint8_t portA = _readReg(MCP23017_GPIOA, ok);
    if (!ok) return 0xFFFF;   // I2C error sentinel

    uint8_t portB = _readReg(MCP23017_GPIOB, ok);
    if (!ok) return 0xFFFF;

    // Invert both bytes: pull-up means ON=LOW in hardware, we report ON=1
    uint8_t sw1 = ~portA;   // DIP Switch 1 state, bit 0 = position 1
    uint8_t sw2 = ~portB;   // DIP Switch 2 state, bit 0 = position 1

    // Pack into 16-bit word: SW1 in low byte, SW2 in high byte
    return (uint16_t)(((uint16_t)sw2 << 8) | sw1);
}

// ── Public: readSwitch1 (Port A only) ────────────────────────────────────────

uint8_t DIPSwitch16::readSwitch1()
{
    bool ok = true;
    uint8_t portA = _readReg(MCP23017_GPIOA, ok);
    if (!ok) return 0xFF;
    return ~portA;   // Invert: ON=LOW hardware → ON=1 returned
}

// ── Public: readSwitch2 (Port B only) ────────────────────────────────────────

uint8_t DIPSwitch16::readSwitch2()
{
    bool ok = true;
    uint8_t portB = _readReg(MCP23017_GPIOB, ok);
    if (!ok) return 0xFF;
    return ~portB;
}

// ── Public: readPosition ──────────────────────────────────────────────────────

bool DIPSwitch16::readPosition(uint8_t switchNum, uint8_t position)
{
    // Validate inputs
    if (position < 1 || position > 8) return false;
    if (switchNum < 1 || switchNum > 2) return false;

    uint8_t val;
    if (switchNum == 1) {
        val = readSwitch1();
    } else {
        val = readSwitch2();
    }

    // position 1 → bit 0, position 8 → bit 7
    uint8_t bitIndex = position - 1;
    return (val >> bitIndex) & 0x01;
}

// ── Public: isConnected ───────────────────────────────────────────────────────

bool DIPSwitch16::isConnected()
{
    _wire.beginTransmission(_address);
    return (_wire.endTransmission() == 0);   // 0 = ACK received
}

// ── Public: setAddress ────────────────────────────────────────────────────────

bool DIPSwitch16::setAddress(uint8_t address)
{
    if (address < 0x20 || address > 0x27) return false;
    _address = address;
    _initialised = false;   // Force re-init on next begin() call
    return true;
}

// ── Private: _writeReg ────────────────────────────────────────────────────────

bool DIPSwitch16::_writeReg(uint8_t reg, uint8_t value)
{
    _wire.beginTransmission(_address);
    _wire.write(reg);      // Register address pointer
    _wire.write(value);    // Value to write
    return (_wire.endTransmission() == 0);  // 0 = success
}

// ── Private: _readReg ─────────────────────────────────────────────────────────

uint8_t DIPSwitch16::_readReg(uint8_t reg, bool &ok)
{
    // First write the register address we want to read from
    _wire.beginTransmission(_address);
    _wire.write(reg);
    if (_wire.endTransmission(false) != 0) {  // false = repeated start
        ok = false;
        return 0x00;
    }

    // Request one byte from the MCP23017
    uint8_t bytesReceived = _wire.requestFrom(_address, (uint8_t)1);
    if (bytesReceived != 1 || !_wire.available()) {
        ok = false;
        return 0x00;
    }

    ok = true;
    return _wire.read();
}
