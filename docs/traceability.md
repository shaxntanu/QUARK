# Requirements traceability

| Requirement | Implementation | Verification | Status |
|---|---|---|---|
| FR-01 local PC metrics | `quark.py:get_system_data` | Source review, Python compile; individual live sensors depend on host hardware | Implemented; partial host-specific test coverage |
| FR-02 NVIDIA GPU metrics | `quark.py:get_nvidia_stats`, NVIDIA parsers | Parser unit tests; no GPU integration test | Implemented when `nvidia-smi` is available |
| FR-03 Codex usage | `CodexAppServer`, `get_codex_data` | Parser unit tests; live account/App Server not exercised in this audit | Implemented; live integration unverified |
| FR-04 bounded serial protocol | `format_ai_packet`, `format_system_packet`, `send_line`; firmware parser | Packet-size, malformed ASCII, and command unit tests; firmware compile | Source/build verified; physical handshake unverified |
| FR-05 switch/button and page selection | `QUARK.ino:updateMode`, `updateButton`, `drawDisplay` | Firmware compile | Implemented; physical controls unverified in this audit |
| FR-06 LEDs and connection acknowledgement | `QUARK.ino:updateStatusLED`, `updateConnectionAcknowledgement` | Firmware compile | Implemented; physical color/sound unverified |
| NFR-01 160-byte line bound | Shared companion/firmware constants, including pre-LF CR bytes | Varied packet-size regression tests; firmware CI compile | Unit/build verified; physical link untested |
| NFR-02 PC telemetry survives Codex failure | Separate Codex polling worker and system loop | Source review only; no long-duration outage test | Implemented; integration behavior unverified |
| NFR-03 bounded polling | `SYSTEM_INTERVAL`, `CODEX_INTERVAL`, retry limits | Source review | Implemented; runtime timing not measured |
| NFR-04 unavailable optional data is non-fatal | Collector fallbacks and parser defaults | Unit tests for malformed and absent values | Partially verified |
| NFR-05 repeatable local build | Pinned Python dependencies, scripts, CI workflow | `scripts/verify.ps1`; hosted CI runs 41468741407 and 41469042013 | Local Windows package and hosted package/firmware builds verified |
| NFR-06 malformed serial input | Strict ASCII decoding, exact command whitelist, firmware line collector | Unit tests for non-ASCII, control bytes, embedded newline, and unknown commands | Unit/build verified; physical fault injection untested |
| NFR-07 App Server response dispatch | `_is_jsonrpc_response`, `CodexAppServer._stdout_reader` | Unit tests for responses, notifications, and server requests | Unit verified; live App Server integration untested |
| NFR-08 canonical local checks | `scripts/verify.ps1`, `scripts/check_docs.py` | Full local Windows verification and CI documentation checker | Local verified; external URLs and anchors are outside the check |

“Implemented” describes source behavior. It does not imply a successful physical or live-service test; those are explicitly marked above. Supporting test results and gaps are in [engineering evidence](engineering-evidence.md).
