/**
 * AllFunctions.ino
 * DIPSwitch16 Library — Complete Function Reference Example
 *
 * Demonstrates every public method in the DIPSwitch16 library:
 *   - Constructor variants (default address, custom address, custom Wire bus)
 *   - begin()
 *   - read()
 *   - readSwitch1() / readSwitch2()
 *   - readPosition()
 *   - isConnected()
 *   - getAddress()
 *   - setAddress()
 *
 * Hardware:
 *   MCP23017 on I2C / Qwiic, address 0x20 (A0, A1, A2 all to GND)
 *   DIP Switch 1 (8-pos) → GPA0–GPA7, common → GND
 *   DIP Switch 2 (8-pos) → GPB0–GPB7, common → GND
 *
 * Serial output at 115200 baud.
 */

#include <Wire.h>
#include <DIPSwitch16.h>

// ─────────────────────────────────────────────────────────────────────────────
// CONSTRUCTOR VARIANTS
// ─────────────────────────────────────────────────────────────────────────────
//
// Option 1 — Default address (0x20), default Wire bus:
//   DIPSwitch16 dips;
//
// Option 2 — Custom address, default Wire bus:
//   DIPSwitch16 dips(0x21);
//
// Option 3 — Custom address AND custom Wire bus (e.g. Wire1 on boards
//            that have a second I2C peripheral):
//   DIPSwitch16 dips(0x22, Wire1);
//
// We use Option 1 here (matches the wiring described above).

DIPSwitch16 dips;   // Default: address 0x20, Wire bus

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

// Print a byte as 8 space-separated bits, MSB first (position 8 → position 1)
void printByte(uint8_t val) {
    for (int b = 7; b >= 0; b--) {
        Serial.print((val >> b) & 1);
        if (b > 0) Serial.print(' ');
    }
}

// Print a 16-bit word as two groups of 8 bits
void print16(uint16_t val) {
    uint8_t hi = (val >> 8) & 0xFF;
    uint8_t lo =  val       & 0xFF;
    Serial.print(F("SW2→ "));
    printByte(hi);
    Serial.print(F("  SW1→ "));
    printByte(lo);
}

void printSectionHeader(const __FlashStringHelper *title) {
    Serial.println();
    Serial.println(F("══════════════════════════════════════════"));
    Serial.println(title);
    Serial.println(F("══════════════════════════════════════════"));
}

// ─────────────────────────────────────────────────────────────────────────────
// setup()
// ─────────────────────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    while (!Serial) { ; }   // Wait for USB CDC on native-USB boards

    Serial.println(F("\n\nDIPSwitch16 — All Functions Demo"));
    Serial.println(F("====================================="));

    // ── 1. begin() ────────────────────────────────────────────────────────────
    // Must be called once from setup().
    // Starts Wire, verifies the device is on the bus, and configures all
    // 16 GPIO pins as inputs with internal pull-ups enabled.
    //
    // Return values:
    //   DIPSWITCH16_OK       (0) — success
    //   DIPSWITCH16_ERR_ADDR (1) — address not in 0x20–0x27 range
    //   DIPSWITCH16_ERR_I2C  (2) — no I2C ACK (device not found / bad wiring)

    printSectionHeader(F("1. begin()"));

    uint8_t result = dips.begin();

    switch (result) {
        case DIPSWITCH16_OK:
            Serial.println(F("  begin() → DIPSWITCH16_OK — device ready."));
            break;

        case DIPSWITCH16_ERR_ADDR:
            Serial.println(F("  begin() → DIPSWITCH16_ERR_ADDR"));
            Serial.println(F("  Address is out of the 0x20–0x27 range."));
            Serial.println(F("  Check the address passed to the constructor."));
            haltWithBlink();
            break;

        case DIPSWITCH16_ERR_I2C:
            Serial.println(F("  begin() → DIPSWITCH16_ERR_I2C"));
            Serial.println(F("  No ACK from MCP23017. Check:"));
            Serial.println(F("    • Qwiic / SDA / SCL wiring"));
            Serial.println(F("    • VCC and GND connections"));
            Serial.println(F("    • A0/A1/A2 jumpers match address 0x20"));
            Serial.println(F("    • RESET pin tied HIGH"));
            haltWithBlink();
            break;
    }

    // ── 2. getAddress() ───────────────────────────────────────────────────────
    // Returns the I2C address the object is currently using.

    printSectionHeader(F("2. getAddress()"));

    uint8_t addr = dips.getAddress();
    Serial.print(F("  Current I2C address: 0x"));
    Serial.println(addr, HEX);   // Expected: 20

    // ── 3. isConnected() ──────────────────────────────────────────────────────
    // Sends a bare I2C probe to the address and returns true if the device ACKs.
    // Useful for runtime health checks without reading any registers.

    printSectionHeader(F("3. isConnected()"));

    if (dips.isConnected()) {
        Serial.println(F("  isConnected() → true  — MCP23017 is responding."));
    } else {
        Serial.println(F("  isConnected() → false — device not found!"));
    }

    // ── 4. setAddress() ───────────────────────────────────────────────────────
    // Changes the I2C address at runtime. Returns true on success, false if
    // the address is outside the 0x20–0x27 range.
    //
    // After setAddress(), call begin() again to re-initialise the new device.
    // Here we demonstrate the call, then restore the original address.

    printSectionHeader(F("4. setAddress()"));

    // Try a valid address
    bool changed = dips.setAddress(0x21);
    Serial.print(F("  setAddress(0x21) → "));
    Serial.println(changed ? F("true (accepted)") : F("false (rejected)"));
    Serial.print(F("  getAddress() now reports: 0x"));
    Serial.println(dips.getAddress(), HEX);   // 21

    // Try an invalid address (out of range)
    changed = dips.setAddress(0x10);
    Serial.print(F("  setAddress(0x10) → "));
    Serial.println(changed ? F("true (accepted)") : F("false — out of range, rejected)"));
    Serial.print(F("  getAddress() still reports: 0x"));
    Serial.println(dips.getAddress(), HEX);   // Still 21

    // Restore original address and re-initialise so the rest of the demo works
    dips.setAddress(0x20);
    dips.begin();   // Re-init after address change
    Serial.println(F("  Restored address 0x20 and called begin() again."));

    Serial.println(F("\n--- setup() complete. Entering loop()... ---\n"));
    delay(2000);
}

// ─────────────────────────────────────────────────────────────────────────────
// loop()
// ─────────────────────────────────────────────────────────────────────────────

uint16_t lastState = 0x0000;   // Used for change-detection demo
uint8_t  loopCount = 0;        // Counts loop iterations

void loop() {
    loopCount++;

    // ── Runtime connection check ───────────────────────────────────────────────
    // isConnected() can be called any time to verify the bus is still alive.
    if (!dips.isConnected()) {
        Serial.println(F("[!] MCP23017 not responding — check wiring."));
        delay(1000);
        return;
    }

    // ─────────────────────────────────────────────────────────────────────────
    // 5. read() — primary API call
    // ─────────────────────────────────────────────────────────────────────────
    // Returns a 16-bit value representing all 16 switch positions.
    //   Bits  [7:0]  = DIP Switch 1 (GPA0=bit0 … GPA7=bit7)
    //   Bits [15:8]  = DIP Switch 2 (GPB0=bit8 … GPB7=bit15)
    //
    // 1 = switch ON (closed / pulled to GND)
    // 0 = switch OFF (open)
    //
    // Returns 0xFFFF on I2C read failure.

    uint16_t allSwitches = dips.read();

    // ─────────────────────────────────────────────────────────────────────────
    // 6. readSwitch1() — read Port A only
    // ─────────────────────────────────────────────────────────────────────────
    // Returns 8-bit value for DIP Switch 1.
    // Equivalent to (allSwitches & 0xFF) but issues its own I2C read.

    uint8_t sw1 = dips.readSwitch1();

    // ─────────────────────────────────────────────────────────────────────────
    // 7. readSwitch2() — read Port B only
    // ─────────────────────────────────────────────────────────────────────────
    // Returns 8-bit value for DIP Switch 2.
    // Equivalent to ((allSwitches >> 8) & 0xFF) but issues its own I2C read.

    uint8_t sw2 = dips.readSwitch2();

    // ─────────────────────────────────────────────────────────────────────────
    // 8. readPosition() — read a single switch position
    // ─────────────────────────────────────────────────────────────────────────
    // Args: switchNum (1 or 2), position (1–8)
    // Returns: true = ON, false = OFF (also false on invalid args)

    bool sw1_pos1 = dips.readPosition(1, 1);   // SW1 position 1
    bool sw1_pos4 = dips.readPosition(1, 4);   // SW1 position 4
    bool sw1_pos8 = dips.readPosition(1, 8);   // SW1 position 8
    bool sw2_pos1 = dips.readPosition(2, 1);   // SW2 position 1
    bool sw2_pos5 = dips.readPosition(2, 5);   // SW2 position 5
    bool sw2_pos8 = dips.readPosition(2, 8);   // SW2 position 8

    // ── Print everything once per loop ────────────────────────────────────────
    Serial.print(F("Loop #"));
    Serial.println(loopCount);

    // read() — full 16-bit
    Serial.print(F("  read()        [Pos 8→1]: "));
    print16(allSwitches);
    Serial.print(F("   (0x"));
    char buf[5];
    sprintf(buf, "%04X", allSwitches);
    Serial.print(buf);
    Serial.println(F(")"));

    // readSwitch1() — 8-bit SW1
    Serial.print(F("  readSwitch1() [Pos 8→1]: "));
    printByte(sw1);
    Serial.print(F("   (0x"));
    if (sw1 < 0x10) Serial.print('0');
    Serial.print(sw1, HEX);
    Serial.println(F(")"));

    // readSwitch2() — 8-bit SW2
    Serial.print(F("  readSwitch2() [Pos 8→1]: "));
    printByte(sw2);
    Serial.print(F("   (0x"));
    if (sw2 < 0x10) Serial.print('0');
    Serial.print(sw2, HEX);
    Serial.println(F(")"));

    // readPosition() — individual positions
    Serial.println(F("  readPosition():"));
    Serial.print(F("    SW1 pos 1 → ")); Serial.println(sw1_pos1 ? F("ON") : F("OFF"));
    Serial.print(F("    SW1 pos 4 → ")); Serial.println(sw1_pos4 ? F("ON") : F("OFF"));
    Serial.print(F("    SW1 pos 8 → ")); Serial.println(sw1_pos8 ? F("ON") : F("OFF"));
    Serial.print(F("    SW2 pos 1 → ")); Serial.println(sw2_pos1 ? F("ON") : F("OFF"));
    Serial.print(F("    SW2 pos 5 → ")); Serial.println(sw2_pos5 ? F("ON") : F("OFF"));
    Serial.print(F("    SW2 pos 8 → ")); Serial.println(sw2_pos8 ? F("ON") : F("OFF"));

    // readPosition() — invalid args (both should return false silently)
    bool badSwitch   = dips.readPosition(3, 1);   // switchNum out of range
    bool badPosition = dips.readPosition(1, 9);   // position out of range
    Serial.println(F("  readPosition() — invalid args (both should be false):"));
    Serial.print(F("    SW3 pos 1 (bad switchNum) → "));
    Serial.println(badSwitch   ? F("true (unexpected!)") : F("false (correct)"));
    Serial.print(F("    SW1 pos 9 (bad position)  → "));
    Serial.println(badPosition ? F("true (unexpected!)") : F("false (correct)"));

    // ── Change detection using read() ─────────────────────────────────────────
    // A common real-world pattern: only act when something changes.
    if (allSwitches != lastState) {
        uint16_t changed = allSwitches ^ lastState;   // Bitmask of flipped bits

        Serial.println(F("  *** CHANGE DETECTED ***"));
        for (uint8_t sw = 1; sw <= 2; sw++) {
            uint8_t changedByte = (sw == 1) ? (changed & 0xFF) : ((changed >> 8) & 0xFF);
            uint8_t currentByte = (sw == 1) ? (allSwitches & 0xFF) : ((allSwitches >> 8) & 0xFF);

            for (uint8_t pos = 1; pos <= 8; pos++) {
                if ((changedByte >> (pos - 1)) & 1) {
                    bool isOn = (currentByte >> (pos - 1)) & 1;
                    Serial.print(F("    SW"));
                    Serial.print(sw);
                    Serial.print(F(" position "));
                    Serial.print(pos);
                    Serial.print(F(" changed → "));
                    Serial.println(isOn ? F("ON") : F("OFF"));
                }
            }
        }
        lastState = allSwitches;
    }

    Serial.println(F("──────────────────────────────────────────"));
    delay(500);
}

// ─────────────────────────────────────────────────────────────────────────────
// Halt on fatal error — blink built-in LED so it's obvious on headless boards
// ─────────────────────────────────────────────────────────────────────────────
void haltWithBlink() {
    Serial.println(F("\n[HALTED] Fix the error above and reset the board."));
    pinMode(LED_BUILTIN, OUTPUT);
    while (true) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(200);
        digitalWrite(LED_BUILTIN, LOW);
        delay(200);
    }
}
