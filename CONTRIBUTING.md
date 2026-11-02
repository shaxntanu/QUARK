# Contributing

QUARK combines a Windows Python companion, ESP32-C3 firmware, and KiCad hardware. Keep changes scoped and describe which side of the USB protocol they affect.

## Local development

1. Install Python 3.12 and the dependencies from `requirements.txt`.
2. Run `py -3.12 -m unittest discover -s tests -v`.
3. Run `py -3.12 -m compileall -q quark.py tests`.
4. For packaging changes, build with `scripts/build_exe.ps1`.
5. For firmware changes, compile `QUARK.ino` for `esp32:esp32:esp32c3:CDCOnBoot=cdc` with the ESP32 core and U8g2 versions recorded in [verification.md](docs/verification.md).
6. For schematic/PCB changes, run KiCad ERC and DRC using the commands in the hardware README. Review ignored checks and fabrication outputs as well as included violations.

## Change expectations

- Add or update tests when changing Python parsing, formatting, or serial behavior.
- Keep the firmware and companion protocol in sync; update the protocol table and packet tests when fields change.
- Clearly label hardware tests that require the physical board. Do not report them as passed from a source build.
- Do not commit local build outputs, account data, logs, credentials, personal file paths, or generated KiCad project-local state.
- Do not change the repository's license without an explicit maintainer decision.

Open a pull request with a short summary, verification commands/results, and any remaining hardware checks. The repository currently has no separate lint or code-format tool configured.
