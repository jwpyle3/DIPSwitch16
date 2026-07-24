# DIPSwitch16

Arduino library for reading **two 8-position DIP switches** via an **MCP23017**
16-bit I/O expander over I²C / Qwiic.

---

## Features

- Returns a **single 16-bit value** — all 16 switch positions at once  
- `1` = switch **ON** (closed), `0` = switch **OFF** (open)  
- Supports all **8 MCP23017 address variants** (0x20 – 0x27)  
- Internal **100 kΩ pull-ups** enabled — no external resistors needed  
- Per-switch (`readSwitch1()` / `readSwitch2()`) and **per-position** queries  
- I²C error detection with meaningful return codes  
- Works on any Arduino-compatible board (AVR, ESP32, SAMD, RP2040 …)

---

## Hardware Wiring

### MCP23017 ↔ Microcontroller (Qwiic / I²C)

```
MCP23017          Board
────────          ─────
VCC       ──→    3.3 V  (or 5 V if your MCU is 5 V)
GND       ──→    GND
SDA       ──→    SDA  (Qwiic Blue)
SCL       ──→    SCL  (Qwiic Yellow)
RESET     ──→    VCC  (tie high)

A0        ──→    GND  ┐
A1        ──→    GND  ├─ default address 0x20
A2        ──→    GND  ┘
```

To use a **different address**, tie the A-pins to VCC/GND as follows:

| A2 | A1 | A0 | Address |
|----|----|----|---------|
| L  | L  | L  | `0x20`  ← default |
| L  | L  | H  | `0x21`  |
| L  | H  | L  | `0x22`  |
| L  | H  | H  | `0x23`  |
| H  | L  | L  | `0x24`  |
| H  | L  | H  | `0x25`  |
| H  | H  | L  | `0x26`  |
| H  | H  | H  | `0x27`  |

### DIP Switches ↔ MCP23017

Each DIP switch pin connects between the **MCP23017 GPIO pin** and **GND**.
The library enables internal pull-ups, so **no external resistors are needed**.

```
DIP Switch 1                 MCP23017
────────────                 ────────
Position 1  ─────────────── GPA0
Position 2  ─────────────── GPA1
Position 3  ─────────────── GPA2
Position 4  ─────────────── GPA3
Position 5  ─────────────── GPA4
Position 6  ─────────────── GPA5
Position 7  ─────────────── GPA6
Position 8  ─────────────── GPA7
Common (COM)─────────────── GND

DIP Switch 2                 MCP23017
────────────                 ────────
Position 1  ─────────────── GPB0
Position 2  ─────────────── GPB1
...
Position 8  ─────────────── GPB7
Common (COM)─────────────── GND
```

> **Logic:** Internal pull-up holds pin HIGH (OFF). When switch closes, pin is
> pulled to GND (ON). The library **inverts** this so `1 = ON, 0 = OFF`.

---

## Installation

1. Download or clone this repository.
2. Copy the `DIPSwitch16` folder into your Arduino `libraries/` directory.
3. Restart the Arduino IDE.

---

## Quick Start

```cpp
#include <Wire.h>
#include <DIPSwitch16.h>

DIPSwitch16 dips;           // Default address 0x20
// DIPSwitch16 dips(0x21); // Non-default address

void setup() {
    Serial.begin(115200);

    if (dips.begin() != DIPSWITCH16_OK) {
        Serial.println("MCP23017 not found!");
        while (true);
    }
}

void loop() {
    uint16_t state = dips.read();
    // Bit 0  = SW1 pos 1  …  Bit 7  = SW1 pos 8
    // Bit 8  = SW2 pos 1  …  Bit 15 = SW2 pos 8

    Serial.println(state, BIN);  // Binary view of all 16 switches
    delay(500);
}
```

---

## API Reference

### Constructor

```cpp
DIPSwitch16(uint8_t address = 0x20, TwoWire &wire = Wire);
```

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `begin()` | `uint8_t` | Initialise. Returns `DIPSWITCH16_OK`, `DIPSWITCH16_ERR_ADDR`, or `DIPSWITCH16_ERR_I2C` |
| `read()` | `uint16_t` | All 16 switch positions. Low byte = SW1, high byte = SW2. `1`=ON |
| `readSwitch1()` | `uint8_t` | DIP Switch 1 only (Port A) |
| `readSwitch2()` | `uint8_t` | DIP Switch 2 only (Port B) |
| `readPosition(sw, pos)` | `bool` | Single position. `sw`=1 or 2, `pos`=1–8. `true`=ON |
| `isConnected()` | `bool` | `true` if MCP23017 is responding on the I²C bus |
| `getAddress()` | `uint8_t` | Current I²C address |
| `setAddress(addr)` | `bool` | Change address (0x20–0x27). Returns `false` if invalid |

### Constants

| Constant | Value | Meaning |
|----------|-------|---------|
| `MCP23017_DEFAULT_ADDR` | `0x20` | Default I²C address |
| `DIPSWITCH16_OK` | `0` | `begin()` succeeded |
| `DIPSWITCH16_ERR_ADDR` | `1` | Address out of range |
| `DIPSWITCH16_ERR_I2C` | `2` | Device not found on bus |

---

## 16-Bit Return Value Layout

```
Bit: 15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
     │S2 │S2 │S2 │S2 │S2 │S2 │S2 │S2 │S1 │S1 │S1 │S1 │S1 │S1 │S1 │S1 │
     │P8 │P7 │P6 │P5 │P4 │P3 │P2 │P1 │P8 │P7 │P6 │P5 │P4 │P3 │P2 │P1 │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
       ├─────────── DIP Switch 2 ───────────┤├─────────── DIP Switch 1 ───────────┤
```

`S1`/`S2` = Switch 1/2, `P1`–`P8` = position. `1` = ON, `0` = OFF.

---

## License

MIT License — free to use, modify, and distribute.
