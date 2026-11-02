# Assumption register

The entries below affect reproducibility or observed behavior. Items marked unverified require confirmation against the target computer or physical parts.

| ID | Assumption | Basis | Impact if false | Validation status | Related component |
|---|---|---|---|---|---|
| A-01 | The companion runs on Windows with Python 3.12 or as the packaged Windows executable. | Windows-specific startup and single-instance behavior; local and CI checks target Windows. | Port discovery, startup, or packaging may fail on other operating systems or runtimes. | Windows 11 and Python 3.12 verified; other platforms unsupported or untested. | Python companion |
| A-02 | The Codex CLI is installed and signed in for the same Windows user that launches QUARK. | The companion starts `codex app-server` locally and does not provide another sign-in flow. | Codex usage is unavailable even while PC telemetry continues. | Documented; no live account test in this audit. | Codex integration |
| A-03 | The App Server interface and usage fields returned by the installed CLI remain compatible with the current parser. | The companion follows current OpenAI interface documentation and tolerates missing values. | Codex readings may be missing or parsing may need an update. | Interface documentation reviewed; live schema compatibility untested. | Codex integration |
| A-04 | The ESP32-C3 enumerates as USB CDC and uses newline-terminated ASCII at 115200 baud. | Firmware FQBN enables CDC on boot; both source sides use the same protocol. | Discovery or telemetry exchange fails. | CI compile and Python protocol tests passed; physical handshake untested. | USB serial |
| A-05 | The selected module matches the ESP32-C3 SuperMini 2x8 footprint. | Carrier footprint and BOM specify 2.54 mm pin pitch and 15.24 mm row centers. | The module may not fit or its pins may not match the carrier. | Unverified for the user's exact clone. | Carrier PCB |
| A-06 | The OLED is an SH1106-compatible 128x64 module wired to the documented I2C pins. | Firmware constructor and J1 pin map. | The display may be blank or use incorrect addressing or signal pins. | Firmware build verified; exact display module is unknown and untested. | OLED |
| A-07 | Only one QUARK companion process owns the selected serial port. | The application uses a Windows single-instance mutex and opens the device itself. | Port contention can prevent discovery or transmission. | Source reviewed; contention behavior was not fault-injected. | Serial transport |
| A-08 | The Codex feed may require network access, while local PC metrics do not. | Codex CLI can communicate with OpenAI services; system collectors are local. | Codex can go offline independently of system telemetry. | Documented design; offline behavior not tested in this audit. | Host application |
| A-09 | The attached ESP32 and local Windows account are trusted. | The USB protocol does not authenticate commands. | Another local process can spoof data or change device mode. | Explicit security boundary, not eliminated. | USB serial |

Where a hardware assumption cannot be confirmed from the repository, the exact part should be identified before a production order. See [hardware references](hardware-references.md).
