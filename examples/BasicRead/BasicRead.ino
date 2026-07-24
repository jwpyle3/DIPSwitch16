/**
 * BasicRead.ino
 * DIPSwitch16 library — basic usage example.
 *
 * Hardware:
 *   MCP23017 connected via I2C / Qwiic (A0, A1, A2 all tied to GND → address 0x20)
 *   DIP Switch 1 (8-pos) → GPA0–GPA7  (each pin: MCP23017 GPIO → switch → GND)
 *   DIP Switch 2 (8-pos) → GPB0–GPB7  (same wiring pattern)
 *
 * The library enables internal pull-ups, so no external resistors are needed.
 */

#include <Wire.h>
#include <DIPSwitch16.h>

// ── Create a DIPSwitch16 object ───────────────────────────────────────────────
// Default address 0x20 is used here. To use a different address, pass it:
//   DIPSwitch16 dips(0x21);
DIPSwitch16 dips;

void setup() {
    Serial.begin(115200);
    while (!Serial) { ; }   // Wait for USB serial on boards that need it

    Serial.println(F("DIPSwitch16 — Basic Read Example"));
    Serial.println(F("=================================="));

    // Initialise the library (starts Wire, configures MCP23017)
    uint8_t status = dips.begin();

    if (status == DIPSWITCH16_OK) {
        Serial.print(F("MCP23017 found at address 0x"));
        Serial.println(dips.getAddress(), HEX);
    } else if (status == DIPSWITCH16_ERR_ADDR) {
        Serial.println(F("ERROR: I2C address out of range (must be 0x20–0x27)"));
        while (true) { ; }   // Halt
    } else if (status == DIPSWITCH16_ERR_I2C) {
        Serial.println(F("ERROR: MCP23017 not found on I2C bus. Check wiring!"));
        while (true) { ; }   // Halt
    }
}

void loop() {
    // ── Read full 16-bit value ────────────────────────────────────────────────
    uint16_t switches = dips.read();

    // ── Split into individual switch bytes ────────────────────────────────────
    uint8_t sw1 = switches & 0x00FF;          // Low byte  = DIP Switch 1
    uint8_t sw2 = (switches >> 8) & 0x00FF;   // High byte = DIP Switch 2

    // ── Print DIP Switch 1 ────────────────────────────────────────────────────
    Serial.print(F("SW1 [8..1]: "));
    for (int bit = 7; bit >= 0; bit--) {
        Serial.print((sw1 >> bit) & 1);
        if (bit > 0) Serial.print(' ');
    }
    Serial.print(F("   (0x"));
    Serial.print(sw1, HEX);
    Serial.println(F(")"));

    // ── Print DIP Switch 2 ────────────────────────────────────────────────────
    Serial.print(F("SW2 [8..1]: "));
    for (int bit = 7; bit >= 0; bit--) {
        Serial.print((sw2 >> bit) & 1);
        if (bit > 0) Serial.print(' ');
    }
    Serial.print(F("   (0x"));
    Serial.print(sw2, HEX);
    Serial.println(F(")"));

    // ── Print the combined 16-bit word ────────────────────────────────────────
    Serial.print(F("Combined:  0x"));
    if (switches < 0x1000) Serial.print('0');  // Pad to 4 hex digits
    if (switches < 0x0100) Serial.print('0');
    if (switches < 0x0010) Serial.print('0');
    Serial.println(switches, HEX);

    // ── Demo: read a single position ─────────────────────────────────────────
    // Is DIP Switch 1, position 3 ON?
    bool pos = dips.readPosition(1, 3);
    Serial.print(F("SW1 pos 3 : "));
    Serial.println(pos ? F("ON") : F("OFF"));

    Serial.println(F("──────────────────────────"));
    delay(500);
}
