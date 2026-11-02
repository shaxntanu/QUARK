# Component inventory

This is a concise, manually maintained inventory of direct runtime/build inputs, not a generated SPDX or CycloneDX SBOM. Transitive Python dependencies and the full Arduino toolchain dependency graph are not enumerated here.

## Python companion

| Component | Role | Version source |
|---|---|---|
| Python | Runtime and tests | Python 3.12 |
| psutil | Local operating-system metrics | `requirements.txt` |
| pyserial | USB serial discovery and transport | `requirements.txt` |
| PyInstaller | Windows executable packaging only | `requirements-build.txt` |

## Firmware and CAD

| Component | Role | Version/evidence |
|---|---|---|
| Arduino ESP32 core | ESP32-C3 firmware build | 3.3.10 used for the verification build |
| U8g2 | SH1106 OLED driver | 2.36.19 used for the verification build |
| KiCad | Schematic/PCB authoring and CLI checks | CLI 10.0.7 used for the verification checks |
| GitHub Actions | Hosted Windows packaging and ESP32-C3 compile | Actions pinned to verified upstream commit SHAs in `.github/workflows/ci.yml` |
| ESP32-C3 SuperMini | Controller module | 2×8 header footprint; clones vary mechanically |
| SH1106 OLED | 128×64 monochrome display | Off-board I²C module |

The board-level parts and quantities are listed in [`hardware/QUARK_SuperMini_Carrier/BOM.csv`](../hardware/QUARK_SuperMini_Carrier/BOM.csv). A future release process should generate a complete software SBOM and record firmware/executable hashes.

## Audit-only tools

Crawl4AI 0.9.5 ran in a temporary research environment. It was not installed into the project and is not a runtime or build dependency.
