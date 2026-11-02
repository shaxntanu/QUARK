# Compatibility matrix

Statuses distinguish source/build checks from physical or live-service verification. A successful compile does not mean that every module clone or display is compatible.

## Software and tools

| Platform or tool | Version | Status | Evidence | Notes |
|---|---|---|---|---|
| Windows desktop companion | Windows 11, build 10.0.26300 | Locally verified | Unit tests, documentation checker, and executable build from `scripts/verify.ps1` | No separate interactive launch or startup-session test was made in this audit. |
| Windows GitHub runner | `windows-latest` | CI verified | Hosted workflow runs recorded in [verification.md](verification.md) | Runner image changes over time. |
| Python | 3.12.10 locally; 3.12 in CI | Verified | Unit tests, compileall, pip check, and packaging | Python 3.13 and later are untested. |
| Linux and macOS companion | None claimed | Unsupported | Source uses Windows-specific startup, mutex, and CLI discovery behavior | No cross-platform test job exists. |
| Codex CLI App Server | Current installed CLI, unpinned | Partially verified | Parsing tests and current official interface documentation | No live account session. App Server methods and usage fields can change independently of this repository. |
| NVIDIA metrics | NVIDIA driver with `nvidia-smi` | Partially verified | Parser tests cover sample and unavailable output | No GPU integration test; non-NVIDIA GPU telemetry is not implemented. |
| KiCad | CLI 10.0.7 | Checked | Recorded ERC/DRC run on checked-in files | Other KiCad versions are untested. Ignored checks remain documented. |
| Arduino CLI | 1.5.1 in hosted CI | CI verified | ESP32-C3 compile job | Arduino CLI was not found on the local command path during this audit. |
| Arduino ESP32 core | 3.3.10 | CI build verified | Firmware compile job | Other core versions are untested. |
| U8g2 | 2.36.19 | CI build verified | Firmware compile job | Other versions are untested. |

## Firmware and hardware

| Board or interface | Version or configuration | Status | Evidence | Notes |
|---|---|---|---|---|
| ESP32-C3 SuperMini target | `esp32:esp32:esp32c3:CDCOnBoot=cdc` | CI build verified | Firmware compile workflow | No board revision or exact SuperMini clone MPN is recorded. Physical fit and behavior are untested. |
| Carrier headers | ESP32-C3 SuperMini 2x8, 2.54 mm pitch, 15.24 mm row centers | Expected fit, unverified | KiCad footprint and hardware documentation | Clone dimensions vary. Measure the intended board before fabrication or assembly. |
| OLED | SH1106, 128x64, hardware I2C | Build configured, hardware untested | U8g2 constructor in firmware | The exact off-board display module is unknown. Other controllers and resolutions are untested. |
| USB serial | 115200 baud, newline ASCII, 160-byte full-line bound | Source and build verified | Python regression suite and firmware CI | Protocol has no version negotiation or authentication. The physical connection was not retested. |
| LEDs and passive piezo | Red/blue LEDs; piezo on GPIO5 | Source and build verified | Firmware compile | Color, brightness, tone, current, and acoustic behavior are untested on hardware. Green GPIO7 is unused. |

The currently published release is `v1.1.0`; the source identifies itself as `1.2.0`. This matrix describes the checked source and CI configuration, not a release support promise.
