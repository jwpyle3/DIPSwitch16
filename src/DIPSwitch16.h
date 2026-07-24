/**
 * DIPSwitch16.h
 * Arduino library for reading two 8-position DIP switches
 * via an MCP23017 I/O expander over I2C (Qwiic).
 *
 * Wiring assumption:
 *   Each DIP switch pin connects MCP23017 GPIO → switch → GND.
 *   Internal pull-ups are enabled, so:
 *     Switch OPEN  (OFF) = pin reads HIGH → bit returns 0
 *     Switch CLOSED (ON) = pin reads LOW  → bit returns 1
 *
 * Returned 16-bit value layout:
 *   Bits [7:0]  = DIP Switch 1  (MCP23017 Port A, GPA0=bit0 … GPA7=bit7)
 *   Bits [15:8] = DIP Switch 2  (MCP23017 Port B, GPB0=bit8 … GPB7=bit15)
 *
 * MCP23017 I2C address selection (A2, A1, A0 hardware pins):
 *   A2 A1 A0 → Address
 *    0  0  0 → 0x20  (default / all addr pins to GND)
 *    0  0  1 → 0x21
 *    0  1  0 → 0x22
 *    0  1  1 → 0x23
 *    1  0  0 → 0x24
 *    1  0  1 → 0x25
 *    1  1  0 → 0x26
 *    1  1  1 → 0x27
 */

#ifndef DIPSWITCH16_H
#define DIPSWITCH16_H

#include <Arduino.h>
#include <Wire.h>

// ── MCP23017 default I2C base address ────────────────────────────────────────
#define MCP23017_DEFAULT_ADDR   0x20

// ── MCP23017 register addresses (IOCON.BANK = 0, power-on default) ───────────
#define MCP23017_IODIRA         0x00  // I/O direction register - Port A
#define MCP23017_IODIRB         0x01  // I/O direction register - Port B
#define MCP23017_IPOLA          0x02  // Input polarity register - Port A
#define MCP23017_IPOLB          0x03  // Input polarity register - Port B
#define MCP23017_GPINTENA       0x04  // Interrupt-on-change enable - Port A
#define MCP23017_GPINTENB       0x05  // Interrupt-on-change enable - Port B
#define MCP23017_GPPUA          0x0C  // Pull-up resistor register - Port A
#define MCP23017_GPPUB          0x0D  // Pull-up resistor register - Port B
#define MCP23017_GPIOA          0x12  // GPIO port register - Port A (read pins)
#define MCP23017_GPIOB          0x13  // GPIO port register - Port B (read pins)
#define MCP23017_OLATA          0x14  // Output latch register - Port A
#define MCP23017_OLATB          0x15  // Output latch register - Port B

// ── Error / status codes returned by begin() ─────────────────────────────────
#define DIPSWITCH16_OK          0     // Initialised successfully
#define DIPSWITCH16_ERR_ADDR    1     // Address out of valid range 0x20–0x27
#define DIPSWITCH16_ERR_I2C     2     // No I2C ACK – device not found


class DIPSwitch16 {
public:

    /**
     * Constructor.
     *
     * @param address  MCP23017 I2C address (0x20–0x27).
     *                 Defaults to MCP23017_DEFAULT_ADDR (0x20).
     * @param wire     TwoWire instance to use (defaults to Wire for Qwiic /
     *                 standard I2C). Pass Wire1 if your board has a second bus.
     */
    DIPSwitch16(uint8_t address = MCP23017_DEFAULT_ADDR, TwoWire &wire = Wire);

    /**
     * Initialise the MCP23017.
     * Call once from setup(). Configures all 16 pins as inputs with
     * internal pull-ups enabled.
     *
     * @return DIPSWITCH16_OK on success, or an error code on failure.
     */
    uint8_t begin();

    /**
     * Read both DIP switches and return a 16-bit status word.
     *
     * Bit layout:
     *   Bit  0 = DIP SW1, position 1  (GPA0)
     *   Bit  7 = DIP SW1, position 8  (GPA7)
     *   Bit  8 = DIP SW2, position 1  (GPB0)
     *   Bit 15 = DIP SW2, position 8  (GPB7)
     *
     * A '1' means the switch is ON (closed / pulled to GND).
     * A '0' means the switch is OFF (open).
     *
     * Returns 0xFFFF on I2C read failure (all bits set – distinguishable
     * from a valid all-on reading only via isConnected()).
     */
    uint16_t read();

    /**
     * Read only DIP Switch 1 (Port A, GPA0–GPA7).
     *
     * @return 8-bit value; bit 0 = position 1, bit 7 = position 8.
     *         A '1' means ON. Returns 0xFF on error.
     */
    uint8_t readSwitch1();

    /**
     * Read only DIP Switch 2 (Port B, GPB0–GPB7).
     *
     * @return 8-bit value; bit 0 = position 1, bit 7 = position 8.
     *         A '1' means ON. Returns 0xFF on error.
     */
    uint8_t readSwitch2();

    /**
     * Read a single switch position from either DIP switch.
     *
     * @param switchNum   1 or 2 – which DIP switch.
     * @param position    1–8    – switch position (1 = leftmost / pin 1).
     * @return true if that position is ON, false if OFF or on invalid args.
     */
    bool readPosition(uint8_t switchNum, uint8_t position);

    /**
     * Test whether the MCP23017 is present and responding on the I2C bus.
     *
     * @return true if device acknowledges its address.
     */
    bool isConnected();

    /**
     * Return the I2C address currently in use.
     */
    uint8_t getAddress() const { return _address; }

    /**
     * Change the I2C address at runtime (before or after begin()).
     * Useful if you need to hot-swap or re-init a device.
     *
     * @param address  New address in range 0x20–0x27.
     * @return true if address is valid and was accepted.
     */
    bool setAddress(uint8_t address);

private:
    TwoWire  &_wire;
    uint8_t   _address;
    bool      _initialised;

    // Low-level register helpers
    bool     _writeReg(uint8_t reg, uint8_t value);
    uint8_t  _readReg(uint8_t reg, bool &ok);
};

#endif // DIPSWITCH16_H
