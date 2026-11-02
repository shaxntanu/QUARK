# Changelog

Changes are grouped by repository source version. The current source identifies the companion as 1.2.0; it has not been published as a GitHub release by this audit.

## Unreleased (1.2.0 source)

- Bound telemetry records to the firmware's 160-byte serial-line limit and reject non-ASCII/control values.
- Count CR bytes toward firmware line limits and reject malformed serial commands without lossy decoding.
- Ignore unsolicited Codex App Server methods in the pending-response queue.
- Added regression tests for full telemetry packets and seven-day trend normalization.
- Added evidence, compatibility, failure-analysis, assumptions, hardware-reference, source, artifact, and AI workflow documentation.
- Pin GitHub Actions to verified commit SHAs and add one-command Windows verification with a repository text checker.
- Added local dependency pins and automated Windows package and firmware compile configuration.

## 1.1.0

See the [published GitHub release](https://github.com/shaxntanu/QUARK/releases/tag/v1.1.0) for the release notes.
