# Security and privacy notes

## Data and trust boundaries

- `psutil` and optional `nvidia-smi` readings are collected locally by the companion.
- Codex usage is obtained by starting the installed `codex app-server` process. That CLI may access OpenAI services under the same Windows user's existing sign-in. QUARK does not implement a second account login flow.
- The companion sends selected values to the attached ESP32 over USB serial. The firmware accepts a small set of serial commands and telemetry records; this local protocol is not an authenticated network protocol.
- The reachability check opens TCP port 443 on `chatgpt.com`. It does not send an HTTP request or verify account access.
- Rotating logs are stored under the current user's local application-data directory. The companion logs Codex CLI stderr lines at INFO level; inspect logs for sensitive text before sharing them.

## Threat model and current safeguards

QUARK assumes the Windows account, Codex CLI installation, and connected USB device are trusted. A process already running as that user can generally inspect or interfere with the companion; this project is not a privilege boundary. The serial parser rejects non-printable/non-ASCII or oversized records, the companion recognizes only known command strings, and the Python packet formatter bounds outgoing records. No credentials, API keys, or tokens are intentionally embedded in the repository.

The audit searched source and documentation for common API-key, GitHub-token, PEM-private-key, and bearer-token patterns and found none. This lightweight search is not a full secret scanner. A read-only CI permission set and Dependabot configuration are being added; they reduce maintenance risk but do not amount to a security audit. No dedicated CodeQL or secret-scanning workflow is configured. Firmware and executable artifacts are unsigned.

## Reporting

For a suspected vulnerability, use GitHub's private vulnerability reporting for the repository if it is enabled. If it is unavailable, contact the maintainer privately through the GitHub profile rather than publishing exploit details in a public issue.
