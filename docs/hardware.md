# Hardware integration

The carrier is designed for an ESP32-C3 SuperMini 2×8 module and connects the board-mounted page button and red/blue LEDs to an off-board SH1106 128×64 OLED, SPDT mode switch, and passive piezo.

## Board and assembly

- Two copper layers, 1.6 mm FR-4; nominal outline 70.104 × 55.372 mm.
- Four 3.2 mm non-plated mounting holes.
- SuperMini headers are 2.54 mm pitch with 15.24 mm between row centers. The nominal module body is 22.52 × 18 mm; clone variants should be measured before ordering.
- J1 is the off-board OLED header; J2 is the mode-switch header; J3 is for a passive piezo only.
- Use a series current-limiting resistor for each status LED. The BOM specifies 330 Ω.
- Do not connect a high-current magnetic buzzer directly to GPIO5.

## Pin map

| Function | ESP32-C3 GPIO | Physical location |
|---|---:|---|
| OLED SDA | 0 | J1, off-board OLED |
| OLED SCL | 1 | J1, off-board OLED |
| Page/refresh button | 3 | SW1 on carrier; closes to GND |
| Red status LED | 4 | D1 on carrier, active high |
| Passive piezo | 5 | J3, off-board piezo to GND |
| Mode switch | 6 | J2; AI/GND or System/open |
| Green LED channel | 7 | Unused; faulty channel |
| Blue status LED | 10 | D2 on carrier, active high |

The firmware uses `INPUT_PULLUP` for button and mode-switch sensing. Verify the schematic's J2 pin order and switch wiring before assembly; the selected mode is determined by whether the COM contact is connected to the AI/GND position or left open for System mode.

## Design files

The full source, local symbol/footprint libraries, BOM, previews, fabrication exports, and convenience firmware image are in [`hardware/QUARK_SuperMini_Carrier`](../hardware/QUARK_SuperMini_Carrier/README.md). The firmware build uses `esp32:esp32:esp32c3:CDCOnBoot=cdc` and `U8G2_SH1106_128X64_NONAME_F_HW_I2C`.

## Validation boundary

The audit re-ran KiCad ERC and DRC on the checked-in files. Both reports had zero included violations, but they contain intentionally ignored checks; details are in [verification.md](verification.md). No new physical continuity, fit, power, display, acoustic, or long-duration test was performed. A zero-violation CAD report is not a fabrication, electrical safety, EMC, or assembly certification.
