# Hardware references

The BOM contains generic parts and a clone-dependent ESP32-C3 SuperMini module. Exact manufacturer part numbers are not recorded for most parts, so this file identifies reference material without claiming that a specific purchased module matches it.

| Component | Exact MPN | Manufacturer or source | Reference | Relevant design use | Limits and uncertainty |
|---|---|---|---|---|---|
| ESP32-C3 SoC family | Not applicable to the carrier as a whole | Espressif Systems | [ESP32-C3 datasheet](https://documentation.espressif.com/esp32-c3_datasheet_en.pdf) | Primary reference for the MCU family used by the module | The carrier uses an assembled SuperMini module with an unknown clone vendor. The PDF body could not be retrieved with Crawl4AI during this audit. No numeric ratings are asserted here. |
| ESP32-C3 SuperMini module | Unknown; clone-dependent | The project PCB library and BOM use the generic module name | [Third-party SuperMini module document](https://dl.artronshop.co.th/ESP32-C3%20SuperMini%20datasheet.pdf) | Reference for nominal module outline and 2x8 header fit | This is not evidence that every clone has the same dimensions, pinout, regulator, USB implementation, or electrical characteristics. Measure the intended module before ordering or assembly. |
| OLED module | Unknown | Generic off-board module | U8g2 driver source: [U8g2 repository](https://github.com/olikraus/u8g2) | Firmware uses the SH1106 128x64 hardware-I2C constructor | The display module MPN, controller revision, address straps, and supply limits are not listed. Other display controllers are untested. |
| Status LEDs and resistors | Unknown | Generic BOM parts | The BOM specifies red and blue LEDs with 330 ohm series resistors. | Status indication | LED color, forward voltage, brightness, and current are not measured. The faulty green channel is unused. |
| Passive piezo | Unknown | Generic off-board part | The BOM and hardware README specify a passive piezo disc on GPIO5. | Connection acknowledgement tones | The exact transducer is unknown. Do not substitute a high-current magnetic buzzer without a suitable driver stage. Acoustic level and current were not measured. |
| Button, switch, capacitors, headers, fasteners | Unknown or footprint-compatible | Generic BOM parts | Part descriptions and footprints in [the BOM](../hardware/QUARK_SuperMini_Carrier/BOM.csv) | Local controls, decoupling, and mounting | Footprint compatibility does not verify a specific supplier part or its electrical rating. |

## Standards and compliance

No regulatory or engineering-standard compliance claim is made. The PCB's ERC and DRC results are CAD rule-check evidence only. They do not establish USB compliance, EMC performance, electrical safety, thermal margins, fabrication quality, or assembled-product certification.

## Before fabrication or electrical qualification

Identify the actual SuperMini and OLED modules, compare their pinouts and dimensions with the project, and review their manufacturer documentation. Measure the assembled carrier's supply behavior and GPIO loads before treating the generic BOM as a production specification. See [hardware integration](hardware.md) and [verification limits](verification.md).
