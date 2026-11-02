# Requirements traceability

| Requirement | Implementation | Verification | Status |
|---|---|---|---|
| FR-01 local PC metrics | `quark.py:get_system_data` | Source review, Python compile; individual live sensors depend on host hardware | Implemented; partial host-specific test coverage |
| FR-02 NVIDIA GPU metrics | `quark.py:get_nvidia_stats`, NVIDIA parsers | Parser unit tests; no GPU integration test | Implemented when `nvidia-smi` is available |
| FR-03 Codex usage | `CodexAppServer`, `get_codex_data` | Parser unit tests; live account/App Server not exercised in this audit | Implemented; live integration unverified |
| FR-04 bounded serial protocol | `format_ai_packet`, `format_system_packet`, `send_line`; firmware parser | Packet-size/ASCII unit tests; firmware compile | Source/build verified; physical handshake unverified |
| FR-05 switch/button and page selection | `QUARK.ino:updateMode`, `updateButton`, `drawDisplay` | Firmware compile | Implemented; physical controls unverified in this audit |
| FR-06 LEDs and connection acknowledgement | `QUARK.ino:updateStatusLED`, `updateConnectionAcknowledgement` | Firmware compile | Implemented; physical color/sound unverified |
| NFR-01 160-byte line bound | Shared companion/firmware constants | Worst-case packet regression tests | Verified in unit tests |
| NFR-02 PC telemetry survives Codex failure | Separate Codex polling worker and system loop | Source review only; no long-duration outage test | Implemented; integration behavior unverified |
| NFR-03 bounded polling | `SYSTEM_INTERVAL`, `CODEX_INTERVAL`, retry limits | Source review | Implemented; runtime timing not measured |
| NFR-04 unavailable optional data is non-fatal | Collector fallbacks and parser defaults | Unit tests for malformed and absent values | Partially verified |
| NFR-05 repeatable local build | Pinned Python dependencies, scripts, CI workflow | Local Windows package and firmware builds; first hosted run 41426096332 passed both jobs; firmware runner now pinned to Ubuntu 24.04 | Local and first hosted run verified; confirm the runner pin on the next run |

“Implemented” describes source behavior. It does not imply a successful physical or live-service test; those are explicitly marked above.
