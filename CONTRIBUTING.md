# Contributing

QUARK combines a Windows Python companion, ESP32-C3 firmware, and KiCad hardware. Keep changes scoped and describe which side of the USB protocol they affect.

## Local development

1. Install Python 3.12, then run `py -3.12 -m pip install -r requirements.txt -r requirements-build.txt`.
2. Run `scripts/verify.ps1` from PowerShell. It checks dependencies, unit tests, Python compilation, repository text and Markdown links, and the Windows executable build.
3. For firmware changes, install Arduino CLI, the ESP32 core, and U8g2 versions recorded in [verification.md](docs/verification.md), then run `scripts/verify.ps1 -IncludeFirmware`.
4. For schematic/PCB changes, run KiCad ERC and DRC using the commands in the hardware README. Review ignored checks and fabrication outputs as well as included violations.

## Change expectations

- Add or update tests when changing Python parsing, formatting, or serial behavior.
- Keep the firmware and companion protocol in sync; update the protocol table and packet tests when fields change.
- Clearly label hardware tests that require the physical board. Do not report them as passed from a source build.
- Do not commit local build outputs, account data, logs, credentials, personal file paths, or generated KiCad project-local state.
- Do not change the repository's license without an explicit maintainer decision.
- Do not add em dashes or account-specific Windows profile paths to repository text.

Open a pull request with a short summary, verification commands/results, and any remaining hardware checks. The repository currently has no separate lint or code-format tool configured.
