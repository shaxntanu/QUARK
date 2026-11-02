# Project identity and requirements

## Purpose

QUARK is a personal engineering prototype that puts selected Windows and Codex CLI usage metrics on a small physical OLED. Its engineering motivation is to make frequently changing local metrics glanceable without keeping a dashboard window in view.

This repository is a hybrid desktop-software and embedded-hardware project. It is not an internet-of-things service: the companion and ESP32 communicate locally over USB, although the Codex CLI can access OpenAI services. No scientific novelty or research contribution is claimed. No Sustainable Development Goal alignment is claimed.

## Engineering problem and gap

Windows system readings and Codex usage are separate sources with different interfaces and update behavior. QUARK connects those local sources to one selectable physical display and keeps system telemetry independent from Codex availability. The project addresses integration, serial robustness, and constrained OLED presentation for this specific setup. It does not claim that existing dashboards or device products are insufficient, and it has not compared QUARK with them.

## Users and use cases

The intended user is a Windows developer who has the Codex CLI installed and signed in, and who can assemble or use the ESP32-C3 SuperMini carrier. Typical use is to check Codex token/allowance summaries or PC resource usage at a glance, with PC telemetry continuing even when Codex data is unavailable.

## Scope

**Implemented:** Windows companion; local OS metrics; optional NVIDIA GPU metrics; Codex App Server polling through the Codex CLI; USB serial discovery/handshake/reconnect; separate Codex and system pages; SH1106 OLED rendering; charts; red/blue status feedback; passive-piezo connection acknowledgement; KiCad design and fabrication exports.

**Not implemented or not established:** cross-platform companion support; network telemetry from the ESP32; remote dashboards; physical calibration or automated hardware-in-the-loop validation; measured latency, power, reliability, or user-study results; a declared software or hardware license.

## Functional requirements

| ID | Requirement | Acceptance evidence |
|---|---|---|
| FR-01 | Read local CPU, memory, system-drive, network, uptime, and available battery data. | Source inspection plus unit/build checks; machine-dependent metrics are not all simulated in tests. |
| FR-02 | Read optional NVIDIA GPU load, temperature, and VRAM when `nvidia-smi` is available. | Parser tests; unavailable readings resolve to `--`. |
| FR-03 | Read Codex usage/allowance through the local `codex app-server` process. | Parsing tests and source inspection; requires a signed-in CLI for live data. |
| FR-04 | Transfer telemetry and user commands over USB serial at 115200 baud. | Protocol tests and firmware compilation; live physical handshake not repeated in this audit. |
| FR-05 | Select Codex or system pages with the physical mode switch and advance/refresh with the button. | Firmware implementation and compile; switch/button electrical operation not revalidated here. |
| FR-06 | Indicate selected-feed state using available red/blue LED channels, and acknowledge a stable connection once. | Firmware implementation and compile; LED/buzzer behavior not hardware-tested in this audit. |

## Nonfunctional requirements

| ID | Requirement | Current implementation/evidence |
|---|---|---|
| NFR-01 | Bound serial records to a fixed frame size. | Printable ASCII; newline included in the 160-byte limit; regression tests cover worst-case field lengths. |
| NFR-02 | Keep system monitoring alive during Codex outages. | Codex polling runs independently from the serial/system loop; no integration test simulates a prolonged outage. |
| NFR-03 | Limit resource/API polling. | System interval is 2 seconds; Codex interval is 60 seconds with bounded retries. |
| NFR-04 | Keep unavailable hardware metrics non-fatal. | Optional GPU, battery, and network-probe values use `--` or status markers. |
| NFR-05 | Rebuild from documented, pinned direct Python dependencies. | Dependency pins and local Windows packaging build are recorded in [verification.md](verification.md). |
| NFR-06 | Reject malformed or oversized serial input without silently changing its contents. | Strict ASCII decoding, command whitelist, firmware line bound, and malformed-input tests. |
| NFR-07 | Keep only responses needed by pending Codex App Server calls in the response queue. | Response-envelope predicate and unit tests for responses, notifications, and server requests. |
| NFR-08 | Provide a repeatable Windows verification command and check internal Markdown links and selected repository text rules. | `scripts/verify.ps1`, `scripts/check_docs.py`, and the CI documentation check. |

## Constraints and assumptions

- The supported companion platform is Windows; the scripts and single-instance mutex use Windows-specific behavior.
- Python 3.12 is the source/runtime build target; a packaged executable can be run without a separate Python install.
- Codex pages depend on the Codex CLI being installed and signed in for the same Windows user.
- GPU metrics require a compatible NVIDIA driver and `nvidia-smi`; other GPU vendors are not queried.
- The firmware targets ESP32-C3 with USB CDC on boot and the U8g2 SH1106 128×64 OLED constructor.
- ESP32-C3 SuperMini clones can differ mechanically; verify module/header fit before a production order.

## Measurable objectives

1. Keep every emitted serial packet within the 160-byte line limit; verified by regression tests.
2. Build the Windows companion executable from the source; verified locally on Windows 11 with Python 3.12.10 and PyInstaller 6.22.3.
3. Compile the ESP32-C3 firmware for the configured CDC-on-boot target; verified locally with Arduino core 3.3.10 and U8g2 2.36.19.
4. Keep unit tests, documentation checks, package build, and firmware compile runnable from a clean checkout; the Windows and firmware jobs have passed in hosted GitHub Actions.
5. Preserve honest validation boundaries: no physical result is claimed without a fresh hardware test.
