# Repository audit — 2026-11-02

## Overview

QUARK is a small hybrid desktop/embedded engineering prototype. The reviewed source tree contains the Python Windows companion, ESP32-C3 Arduino firmware, KiCad carrier design and manufacturing exports, packaging/startup scripts, and a Python unit suite. The latest published GitHub release is `v1.1.0`; the companion source identifies itself as `1.2.0`.

## Findings before this audit

- Core code and hardware existed, but project identity, interfaces, data collection, limitations, verification boundaries, and contributor/security guidance were spread across the README or absent.
- Direct Python dependencies were not pinned and the repository had no automated test/build workflow or dependency update configuration.
- The serial limit was enforced at write time, but packet builders could create an overlength `AI` or `SYS` record when many optional metrics were present. The Codex write path could then enter its reconnect/retry path.
- The hardware README reported zero included violations without listing ignored ERC/DRC checks.
- No license file was present. No research novelty, measured benchmark, or recent physical validation evidence was present in this audit.

## Changes made

- Bounded ASCII packet construction to the shared 160-byte complete-line limit and added regression tests for worst-case values, controls/non-ASCII input, and seven-day normalization.
- Added architecture, requirements, methodology, hardware integration, verification, security, traceability, SBOM inventory, roadmap, limitations, research notes, and two current-state ADRs.
- Added a static Anatomy-style system figure based on the implemented host/USB/ESP32/OLED/control path and referenced it from the README.
- Pinned direct Python dependencies, added Windows test/package and ESP32-C3 compile jobs, and enabled weekly Dependabot updates.
- Clarified that the CAD reports ignore 4 ERC checks and 7 DRC checks.
- Added contributor, security, citation, and changelog metadata without selecting a license or creating a new release.

## Verification evidence

- Python suite: 14/14 tests passed; `compileall` passed; `pip check` passed.
- Windows one-file package: built locally, 9,839,344 bytes.
- ESP32-C3 firmware: compile passed; 370,044 bytes application flash and 17,304 bytes global RAM.
- KiCad CLI 10.0.7: ERC 0 included violations; PCB DRC 0 included violations, 0 unconnected items, 0 parity issues. Reports also list ignored checks.
- Personal-path scan: no account-specific absolute path was found in the working tree outside `.git`; the searched commit history also had no match.
- The first GitHub Actions run (41426096332) passed both jobs. The firmware runner is now pinned to Ubuntu 24.04 and needs one follow-up run; the action emitted a Node.js 20-to-24 runtime warning.

Details and limitations are in [verification.md](verification.md). No live Codex session, connected hardware, long-duration run, or physical measurement was used during this audit.

## Maturity assessment

Scale: **1** = source exists with limited repeatable checks; **2** = local tests/builds and evidence are repeatable; **3** = hosted CI is green and a hardware acceptance run is recorded; **4** = releases have reproducible provenance and measured bench results; **5** = stable field validation and maintenance practice.

- **Initial: 1/5.** Source, tests, and hardware outputs existed, but repeatability and evidence boundaries were underdocumented.
- **After changes: 2/5.** Local tests, firmware/package builds, and CAD reports are recorded, dependencies are pinned, and both jobs passed once on hosted CI. Physical acceptance evidence is still pending, and the Ubuntu 24.04 runner pin needs a hosted rerun.

This score is an engineering readiness summary, not a scientific or industry certification.

## Remaining maintainer decisions

1. Choose and add the intended license before inviting reuse or redistribution.
2. Confirm hosted CI after the Ubuntu 24.04 runner pin, and watch for an upstream Arduino setup action release that removes the Node.js runtime warning.
3. Run the physical acceptance checklist in [roadmap.md](roadmap.md) and record actual results.
4. Decide whether to publish the current 1.2.0 source as a release, with artifact hashes and build provenance.
