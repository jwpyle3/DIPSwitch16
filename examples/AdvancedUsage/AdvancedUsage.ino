/**
 * AdvancedUsage.ino
 * DIPSwitch16 library — advanced usage example.
 *
 * Demonstrates:
 *   1. Using a non-default I2C address (0x21, A0 tied HIGH)
 *   2. Change detection – only print when switches change
 *   3. Individual position queries
 *   4. Handling I2C errors gracefully
 */

#include <Wire.h>
#include <DIPSwitch16.h>

// ── Use address 0x21 (A0 pin tied HIGH, A1 and A2 tied to GND) ───────────────
DIPSwitch16 dips(0x21);

uint16_t lastState = 0xFFFF;   // Track previous switch state for change detect

void setup() {
    Serial.begin(115200);
    while (!Serial) { ; }

    Serial.println(F("DIPSwitch16 — Advanced Usage Example"));
    Serial.println(F("======================================"));

    uint8_t status = dips.begin();

    switch (status) {
        case DIPSWITCH16_OK:
            Serial.print(F("Initialised. I2C address: 0x"));
            Serial.println(dips.getAddress(), HEX);
            break;

        case DIPSWITCH16_ERR_ADDR:
            Serial.println(F("Bad address! Valid range: 0x20–0x27"));
            halt();
            break;

        case DIPSWITCH16_ERR_I2C:
            Serial.println(F("Device not found. Check A0/A1/A2 jumpers and wiring."));
            halt();
            break;

        default:
            Serial.println(F("Unknown error."));
            halt();
    }

    // Force an initial print on first loop iteration
    lastState = ~dips.read();
}

void loop() {
    // Bail out if I2C device is no longer responding
    if (!dips.isConnected()) {
        Serial.println(F("WARNING: MCP23017 disconnected!"));
        delay(1000);
        return;
    }

    uint16_t current = dips.read();

    // ── Only act when something has changed ──────────────────────────────────
    if (current != lastState) {
        uint16_t changed = current ^ lastState;   // Bits that flipped

        // ── Print full status ─────────────────────────────────────────────────
        printSwitchTable(current);

        // ── Report which positions changed ────────────────────────────────────
        Serial.println(F("Changes:"));
        for (uint8_t sw = 1; sw <= 2; sw++) {
            uint8_t changedByte = (sw == 1)
                                    ? (changed & 0x00FF)
                                    : ((changed >> 8) & 0xFF);
            uint8_t currentByte = (sw == 1)
                                    ? (current & 0x00FF)
                                    : ((current >> 8) & 0xFF);

            for (uint8_t pos = 1; pos <= 8; pos++) {
                if ((changedByte >> (pos - 1)) & 1) {
                    bool isOn = (currentByte >> (pos - 1)) & 1;
                    Serial.print(F("  SW"));
                    Serial.print(sw);
                    Serial.print(F(" pos "));
                    Serial.print(pos);
                    Serial.print(F(" → "));
                    Serial.println(isOn ? F("ON") : F("OFF"));
                }
            }
        }
        Serial.println();

        lastState = current;
    }

    delay(50);   // 50 ms poll — fast enough for tactile switches
}

// ── Helper: print a formatted switch table ────────────────────────────────────

void printSwitchTable(uint16_t state)
{
    uint8_t sw1 = state & 0x00FF;
    uint8_t sw2 = (state >> 8) & 0xFF;

    Serial.println(F("Pos  : 8  7  6  5  4  3  2  1"));
    Serial.println(F("─────:────────────────────────"));

    Serial.print(F("SW 1 : "));
    for (int b = 7; b >= 0; b--) {
        Serial.print(((sw1 >> b) & 1) ? F("●  ") : F("○  "));
    }
    Serial.println();

    Serial.print(F("SW 2 : "));
    for (int b = 7; b >= 0; b--) {
        Serial.print(((sw2 >> b) & 1) ? F("●  ") : F("○  "));
    }
    Serial.println();

    Serial.print(F("16-bit value: 0x"));
    char buf[5];
    sprintf(buf, "%04X", state);
    Serial.println(buf);
    Serial.println(F("─────────────────────────────────"));
}

void halt() {
    while (true) { delay(1000); }
}
