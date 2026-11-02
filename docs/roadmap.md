# Roadmap proposals

These are suggested future tasks, not commitments or implemented functionality.

## Near term

- Decide and document the intended license before advertising QUARK as open source or accepting redistributed builds.
- Update the Arduino CLI setup action when upstream provides a Node.js 24-compatible release; rerun CI after toolchain or dependency changes.
- Run and record a physical acceptance checklist: SuperMini fit, USB CDC handshake, each switch/button action, each OLED page, red/blue states, and one acknowledgement chime per stable connection.
- Add an opt-in hardware-in-the-loop serial test fixture if the device is regularly available.
- Reconcile the source version string (`1.2.0`) with the next published release and attach build provenance to that release.

## Later

- Add deterministic identifiers to KiCad generators and verify repeatable regeneration before relying on regenerated CAD in reviews.
- Evaluate whether automated Courtyard/DFM checks can be enabled without creating noisy false positives; document the reason for each exception.
- Consider adding an adapter for non-NVIDIA GPU metrics if there is a concrete supported API and test machine.
- Add a lightweight telemetry capture/replay fixture so Codex and system outage transitions can be tested without a live account.
- Measure serial/UI latency, power draw, and long-run stability on actual hardware before publishing quantitative performance claims.
