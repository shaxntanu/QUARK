# Repository audit: 2026-11-02

## Overview

QUARK is a hybrid Windows companion, ESP32-C3 firmware, and KiCad carrier-board project. The Python process collects local PC metrics and Codex usage through the locally installed Codex App Server, then sends bounded ASCII telemetry over USB serial. The firmware selects an OLED page and drives local indicators and controls. The project is a personal engineering prototype, not a research publication or hosted service.

The source identifies itself as version `1.2.0`; the latest published release remains `v1.1.0`. The repository has no `LICENSE` file, so public visibility does not specify reuse or redistribution terms.

## Findings and changes

- The serial input path previously discarded invalid bytes during decoding. The companion now decodes strictly as ASCII, rejects control and embedded line characters, and accepts only known commands. The firmware counts CR bytes against the line limit and discards oversized or invalid input through LF.
- The App Server reader previously queued unsolicited JSON messages even though the request path consumes only responses. It now queues response objects and ignores method notifications and requests.
- CI actions are pinned to upstream release commit SHAs. The workflow has read-only contents permission, and Dependabot checks Python and GitHub Actions updates weekly.
- A local Markdown and text checker validates relative Markdown targets and rejects em dashes and account-specific Windows profile paths in UTF-8 text files. It runs in CI.
- A Windows verification script runs dependency checks, the Python suite, bytecode compilation, documentation checks, and the executable package build. An optional flag adds the ESP32-C3 compile when Arduino CLI and packages are installed.
- Added engineering evidence, research sources, hardware references, compatibility, assumptions, failure analysis, generated-artifact provenance, and an AI skill/tool registry. Existing architecture, requirements, methodology, security, limitations, traceability, SBOM, and roadmap documents were updated to match the implementation.

## Applicable capability gates

| Gate | Status | Evidence or reason |
|---|---|---|
| Windows software and packaging | Activated | Unit tests, one-file executable build, and Windows CI. |
| Embedded firmware and serial interface | Activated | Boundary tests and hosted ESP32-C3 compile; no physical hardware result. |
| PCB and electronics | Activated | KiCad ERC/DRC evidence and a hardware reference ledger; exact purchased clone parts remain unknown. |
| Supply chain and CI | Activated | Direct dependency pins, action SHA pins, read-only workflow token, and Dependabot update configuration. Dependabot alerts are disabled for this repository. |
| Token-efficient agent stack | Fallback applied | None of the five named tools was present in the active tool list or command path. Targeted retrieval and manual minimization were used without adding dependencies. |
| Advanced testing | Partially activated | Malformed and boundary cases are covered. Fuzzing, mutation testing, hardware fault injection, and concurrency stress were not performed. |
| Research evidence | Partially activated | Crawl4AI 0.9.5 and official documentation informed targeted decisions. Some pages and a datasheet PDF could not be retrieved. |
| Performance and observability | Partially activated | Existing rotating logs, polling intervals, and retry bounds were reviewed. No profiling or benchmark was performed. |
| Accessibility | Partially applicable | The device has a monochrome OLED, tactile input, and LEDs. No formal accessibility standard applies or was evaluated; physical controls and OLED use remain untested. |
| Documentation-as-code | Activated | Local Markdown links, em dashes, and profile-specific paths are checked by a local script and CI. External links and anchors are not automatically validated. |
| Artifact provenance and compatibility | Activated | Generated outputs and tested versus untested platforms are listed in dedicated records. |
| SDG, scientific novelty, and benchmark claims | Skipped | No evidence supports an impact, novelty, or comparative performance claim. |
| Web dashboard, cloud service, ML, ManimGX animation | Skipped | These capabilities are not part of the implemented project. A static SVG and Mermaid diagram are sufficient. |

## Verification summary

- Python 3.12.10: 17 unit tests passed; `compileall` and `pip check` passed.
- Windows package: `QUARK.exe` built successfully at 9,840,202 bytes; it was not launched in an interactive user session.
- Firmware: ESP32-C3 compile passed in hosted CI after the serial boundary changes. Current run details are linked from [verification.md](verification.md).
- CAD: KiCad CLI 10.0.7 reported zero included ERC/DRC violations, zero unconnected items, and zero schematic-parity issues. Four ERC and seven DRC checks were ignored and remain disclosed.
- Documentation: relative Markdown targets and repository text policy passed the local checker. It does not verify external URLs or anchors.
- Privacy: the text checker found no account-specific Windows profile path in the checked repository text. The branch history scan found no profile path pattern. No secret scanner was run.

## Remaining limitations

- No fresh hardware-in-the-loop run, physical display check, LED check, button/switch test, buzzer test, electrical measurement, fit check, or long-duration run was performed.
- No live signed-in Codex App Server session or network-interruption test was performed.
- Exact SuperMini, OLED, LED, piezo, and other generic BOM part numbers are not known.
- No measured latency, throughput, power, thermal, RF/EMC, reliability, or user-study results are published.
- No transitive dependency lockfile, full vulnerability scan, CodeQL workflow, secret-scanning workflow, signed firmware, or build attestation exists.
- The checked-in merged firmware image was not reproduced bit-for-bit by a previous local build.
- The project has no license and source version `1.2.0` has not been published as a release.

## Maturity assessment

Scale: **1** means source exists with limited repeatable checks; **2** means local tests/builds and evidence are repeatable; **3** means hosted CI is green and a hardware acceptance run is recorded; **4** means releases have reproducible provenance and measured bench results; **5** means stable field validation and maintenance practice.

- **Initial: 1/5.** Source, tests, and hardware outputs existed, but repeatability and evidence boundaries needed improvement.
- **After this audit: 2/5.** Local tests, package build, firmware CI, CAD evidence, documentation checks, and source records are repeatable. Physical acceptance evidence is pending.

The score is a project readiness summary, not a certification.

## Recommended maintainer actions

1. Identify the exact SuperMini and OLED modules before ordering a production batch.
2. Run and record the physical acceptance checklist in [roadmap.md](roadmap.md).
3. Decide and add the intended license.
4. Review action and dependency updates as Dependabot proposes them; use an external vulnerability scanner if maintaining public releases.
5. Reconcile source version `1.2.0` with a future release and publish checksums and provenance if binaries are distributed.
