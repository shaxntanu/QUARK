# Verification record

Results below were collected on 2026-11-02 from the current `main` source. They combine local reproducibility evidence with hosted CI runs; they do not claim physical validation.

## Software and firmware

| Check | Environment | Result |
|---|---|---|
| Python unit suite | Python 3.12.10 | 17 tests passed. Includes parsing, missing-value behavior, JSON-RPC message filtering, malformed command input, and serial packet bounds. |
| Python bytecode compile | Python 3.12.10 | `compileall` passed for the companion, tests, and scripts. |
| Documentation checker | Python 3.12.10 | 66 UTF-8 text files and 29 Markdown files passed relative-link, em-dash, and account-specific Windows profile path checks. External links and Markdown anchors are not checked. |
| Windows executable package | Windows 11, Python 3.12.10, PyInstaller 6.22.3 | `scripts/verify.ps1` built a one-file `QUARK.exe` at 9,840,202 bytes. It was not launched in a separate user session. |
| ESP32-C3 sketch | Arduino CLI 1.5.1, ESP32 core 3.3.10, U8g2 2.36.19, `CDCOnBoot=cdc` | Hosted firmware compile passed after the serial boundary changes. This machine did not have `arduino-cli` on PATH for a new local compile. |
| Hosted GitHub Actions | Runs [41468741407](https://github.com/shaxntanu/QUARK/actions/runs/41468741407), [41469042013](https://github.com/shaxntanu/QUARK/actions/runs/41469042013), and [41469616534](https://github.com/shaxntanu/QUARK/actions/runs/41469616534) | Windows package/test and ESP32-C3 compile passed. Run 41469042013 verifies the SHA-pinned actions. Run 41469616534 also runs the documentation checker. |

The locally built merged 4 MB image and checked-in merged firmware image are **not bit-identical**. Their SHA-256 digests are `20B413C3E7E5EC4A698DCF356F5AB2AE3BFEBF2BE9BDF71AA0AB9DFB52DBB761` (local build) and `F294DB709940EF8ADDC76F1F1B0F082A535058B495AF5260D6E658B938DDA73F` (checked-in). The difference was not diagnosed; do not treat the checked-in binary as reproduced by this compile.

Run the full Windows local verification workflow with:

```powershell
.\scripts\verify.ps1
```

Add `-IncludeFirmware` to compile locally when Arduino CLI and the pinned packages are installed. Firmware compilation is also run by hosted CI. The executable build uses `scripts/build_exe.ps1`.

## KiCad checks

KiCad CLI 10.0.7 ran ERC and PCB DRC on the checked-in design. Both reports included error and warning severities.

| Check | Included findings | Ignored checks |
|---|---:|---:|
| Schematic ERC | 0 violations | 4: single global label, four-way junction, simulation-model issue, footprint-filter mismatch |
| PCB DRC with schematic parity and refilled zones | 0 violations; 0 unconnected items; 0 parity issues | 7: missing courtyard, track endpoint not centered on via, tuning-profile geometry, footprint-filter mismatch, PTH inside courtyard, NPTH inside courtyard, footprint/pad-type mismatch |

The ignored checks are part of the report and should not be hidden when interpreting the result. In particular, zero included violations does not mean every courtyard/assembly/DFM check passed.

## Not tested in this audit

- A live Codex CLI session, account-data availability, internet failure/recovery, or quota accuracy.
- A serial handshake with a connected board, physical LED colors, OLED layout, button/switch operation, or buzzer sound.
- Power consumption, electrical margins, RF/EMC, thermal behavior, fabrication fit, and long-duration reliability.
- Hardware-in-the-loop testing, live Codex use, internet failure/recovery, and physical readings.
- Why the checked-in firmware image differs from a previous local build.
- External Markdown link reachability, Markdown anchor validity, full secret scanning, and dependency vulnerability results.
- Code coverage, static-analysis score, latency, or throughput benchmark. No such measured results are published.

The latest GitHub release is `v1.1.0`; the current source's companion version string is `1.2.0`. No new release or tag is created by this audit.
