# DIPSwitch16 Breakout — Fabrication Package

A companion PCB for the **DIPSwitch16** Arduino library: an MCP23017 I²C
expander reading two 8-position DIP switches, with a 4-position DIP switch
for I²C address selection + power-LED enable, and dual Qwiic connectors for
daisy-chaining.

```
Board:  58.5 × 55.5 mm, 2-layer, 1.6 mm FR-4, 4 mm rounded corners
Copper: 1 oz  ·  Min trace/clearance used: 0.40 / 0.25 mm (easy for any fab)
Finish: HASL or ENIG — either is fine
```

## Files

| File | Layer |
|---|---|
| `DIPSwitch16.GTL` / `.GBL` | Top / bottom copper |
| `DIPSwitch16.GTS` / `.GBS` | Top / bottom solder mask |
| `DIPSwitch16.GTO` | Top silkscreen |
| `DIPSwitch16.GKO` | Board outline |
| `DIPSwitch16.TXT` | Plated drills (vias 0.4 mm, TH pins 0.9 mm) |
| `DIPSwitch16-NPTH.TXT` | Non-plated drills (4× M3 mounting holes, 3.2 mm, centred 4 mm from each corner) |

Upload the whole zip to JLCPCB / PCBWay / OSH Park with default 2-layer
settings. **Before ordering, open the zip in the fab's online Gerber viewer
(or `gerbv`) and eyeball every layer** — these files were generated
programmatically rather than exported from KiCad, so a one-minute visual
check is cheap insurance.

## How the circuit works

- **U1 MCP23017 (SPDIP-28, narrow 0.3" rows — socket recommended).** GPB0–7 → SW2 (left),
  GPA0–7 → SW1 (right). Each switch position shorts its GPIO to GND when ON;
  the library enables the chip's internal pull-ups, so there are no external
  resistors on the switch lines — exactly the wiring in the library README.
- **SW3 positions 1–3 (ADDR):** ON ties A0/A1/A2 to 3.3 V; R1–R3 (10 kΩ)
  pull them low when OFF. All OFF = address **0x20** … all ON = **0x27**,
  matching the table in `DIPSwitch16.h`.
- **SW3 position 4 (LED):** feeds the power LED through R4 (1 kΩ).
  ON = LED lit when powered, OFF = always dark. Purely user preference.
- **J1/J2 Qwiic (JST SH 1 mm):** pin 1 GND, 2 3V3, 3 SDA, 4 SCL. Two
  connectors so you can chain the board mid-bus.
- **/RESET** is tied to 3V3; **C1** (100 nF) decouples VDD.

## Bill of Materials

| Ref | Part | Package | Qty |
|---|---|---|---|
| U1 | MCP23017-E/SP | SPDIP-28, 0.3" / 7.62 mm rows + narrow socket | 1 |
| SW1, SW2 | 8-position DIP switch | DIP-16, 2.54 mm | 2 |
| SW3 | 4-position DIP switch | DIP-8, 2.54 mm | 1 |
| J1, J2 | JST SH 4-pin side-entry (Qwiic), SM04B-SRSS-TB compatible | SMD | 2 |
| R1–R3 | 10 kΩ | 0805 | 3 |
| R4 | 1 kΩ | 0805 | 1 |
| D1 | LED (any color) | 0805 | 1 |
| C1 | 100 nF | 0805 | 1 |

## Assembly notes

1. **SW1 mounts "upside-down":** its position-1 marking goes toward the
   **bottom** edge of the board (the silkscreen dot marks position 1). This
   keeps GPA0 = position 1 with the shortest traces. SW2's position 1 is at
   the top, dot likewise marked.
2. **Every part is labelled on the silkscreen** — the passives sit in one
   row (left to right: R4 1K, D1 LED, R1 10K, R2 10K, R3 10K) with the
   reference and value printed above each part. R1/R2/R3 are the A0/A1/A2
   pulldowns in that order. **D1 orientation:** the silkscreen bar at the
   top of the D1 outline is the **cathode** (GND side); the anode (bottom
   pad) faces the board edge.
3. **Verify the JST SH footprint** against the datasheet of the exact
   connector you buy before ordering — SH-series clones vary slightly.
4. Vias are tented (mask-covered); only pads are exposed.

## Electrical notes

- **No I²C pull-up resistors on this board** — deliberate. Qwiic hosts
  (and most dev boards) already provide bus pull-ups, and stacking more on
  every chained device over-loads the bus. If you use this board with a bare
  MCU that has none, add ~4.7 kΩ from SDA and SCL to 3.3 V somewhere on the bus.
- 3.3 V logic. The MCP23017 tolerates 5 V, but Qwiic is a 3.3 V standard —
  power it from the Qwiic cable and everything just works with your
  ESP32-S3 Metro.
- Address pins are actively driven (3V3 via switch, GND via pulldown), so
  readings are glitch-free; change the address only with power off, then
  match it in the sketch: `DIPSwitch16 dips(0x21);` etc.
