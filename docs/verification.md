# Verification record

Results below were collected on 2026-11-02 from the current `main` source. They combine local reproducibility evidence with one hosted CI run; they do not claim physical validation.

## Software and firmware

| Check | Environment | Result |
|---|---|---|
| Python unit suite | Python 3.12.10 | 14 tests passed. Includes parsing, missing-value behavior, seven-day normalization, ASCII sanitization, and maximum packet-length regressions. |
| Python bytecode compile | Python 3.12.10 | `python -m compileall -q quark.py tests` passed. |
| Windows executable package | Windows 11, PyInstaller 6.22.3 | One-file `QUARK.exe` built successfully in an external temporary directory; 9,839,344 bytes. It was not launched as an end-user session. |
| ESP32-C3 sketch | Arduino CLI 1.5.1, ESP32 core 3.3.10, U8g2 2.36.19, `CDCOnBoot=cdc` | Compile succeeded: 370,044 bytes of 1,310,720-byte application flash (28%); 17,304 bytes of 327,680-byte global RAM (5%). |
| Hosted GitHub Actions | Runs [41426096332](https://github.com/shaxntanu/QUARK/actions/runs/41426096332) and [41426685086](https://github.com/shaxntanu/QUARK/actions/runs/41426685086) | Both Windows package/test and ESP32-C3 compile jobs passed in both runs. Run 41426685086 verifies the firmware runner pinned to Ubuntu 24.04. GitHub still warns that `arduino/setup-arduino-cli@v2.0.0` targets Node.js 20 while runners provide Node.js 24; the action passed. No newer upstream release was available at this check. |

The locally built merged 4 MB image and checked-in merged firmware image are **not bit-identical**. Their SHA-256 digests are `20B413C3E7E5EC4A698DCF356F5AB2AE3BFEBF2BE9BDF71AA0AB9DFB52DBB761` (local build) and `F294DB709940EF8ADDC76F1F1B0F082A535058B495AF5260D6E658B938DDA73F` (checked-in). The difference was not diagnosed; do not treat the checked-in binary as reproduced by this compile.

Run the Python checks with:

```powershell
py -3.12 -m unittest discover -s tests -v
py -3.12 -m compileall -q quark.py tests
```

The executable build uses `scripts/build_exe.ps1`. Firmware compilation uses the command in the root README.

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
- A hosted GitHub Actions run after a future Python, Arduino CLI, ESP32 core, or library dependency update.
- Why the checked-in firmware image differs from the local build.
- Code coverage, static-analysis score, latency, or throughput benchmark. No such measured results are published.

The latest GitHub release is `v1.1.0`; the current source's companion version string is `1.2.0`. No new release or tag is created by this audit.
