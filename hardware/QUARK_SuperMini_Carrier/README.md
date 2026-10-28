# QUARK ESP32-C3 SuperMini Carrier

KiCad source for a two-layer carrier board that connects QUARK's ESP32-C3 SuperMini, OLED, mode switch, page button, passive piezo, and red/blue status LEDs.

## Design files

- `QUARK_SuperMini_Carrier.kicad_pro` — KiCad project.
- `QUARK_SuperMini_Carrier.kicad_sch` — schematic.
- `QUARK_SuperMini_Carrier.kicad_pcb` — routed PCB.
- `QUARK.kicad_sym`, `QUARK.pretty/`, and the library tables — project-local symbol and footprint libraries.
- `BOM.csv` — board and off-board parts.
- `firmware/QUARK_ESP32-C3_SuperMini_CDC.bin` — merged 4 MB ESP32-C3 flash image with USB CDC on boot enabled.
- `fabrication/` — Gerbers, Excellon drill files, and drill maps.
- `preview/` — schematic and board preview images.
- `scripts/generate_schematic.py` and `scripts/generate_board.py` — source generators.

## Board and module fit

The carrier outline is 70.104 × 55.372 mm, 1.6 mm thick, with four 3.2 mm non-plated M3 holes. The module footprint is for an ESP32-C3 SuperMini with two 8-pin rows on 2.54 mm pitch and 15.24 mm between row centers. Fit two 1×8 female socket strips to the carrier, then plug the module in with its USB-C connector toward the board's top edge. The nominal module outline is 22.52 × 18 mm per the [module datasheet](https://dl.artronshop.co.th/ESP32-C3%20SuperMini%20datasheet.pdf); SuperMini clones vary, so compare the module and header spacing before ordering a batch.

## Connectors and assembly

All board connectors use 2.54 mm pitch. Connector pin 1 is identified on the silkscreen.

| Ref. | Pin order | Connection |
|---|---|---|
| J1 | GND, 3V3, SCL, SDA | External 128×64 OLED; SCL is GPIO1 and SDA is GPIO0. |
| J2 | AI, COM, SYS OPEN | SPDT mode switch: connect COM to AI/GND for AI mode; leave COM open for System mode (`INPUT_PULLUP`). |
| J3 | GPIO5, GND | Passive piezo disc only. |
| SW1 | — | Page/refresh button from GPIO3 to GND. |
| D1, D2 | — | Red and blue active-high status LEDs, each with a 330 Ω series resistor. |

The OLED module and mode switch are off-board. J1–J3 are plated header footprints; install pin headers or solder leads as needed. Do not connect a high-current magnetic buzzer directly to GPIO5. The ESP32-C3 SuperMini provides the USB interface and board-level power regulation.

## Fabrication

The `fabrication/` folder contains copper, solder-mask, silkscreen, paste, and outline Gerbers, plus separate PTH and NPTH Excellon files and drill maps. Upload [`QUARK_SuperMini_Carrier_gerbers.zip`](fabrication/QUARK_SuperMini_Carrier_gerbers.zip) to the board fabricator, or upload the individual Gerber and drill files. The board uses two copper layers and a 1.6 mm FR-4 stackup.

## Validation

The checked-in design was exported with KiCad CLI. ERC reports zero schematic violations; PCB DRC reports zero violations, zero unconnected items, and zero schematic-parity issues. Re-run checks after edits with KiCad 10:

```powershell
kicad-cli sch erc QUARK_SuperMini_Carrier.kicad_sch
kicad-cli pcb drc --schematic-parity --refill-zones QUARK_SuperMini_Carrier.kicad_pcb
```
