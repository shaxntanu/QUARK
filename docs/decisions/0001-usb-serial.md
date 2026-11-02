# ADR 0001: USB serial as the host-device link

## Status

Accepted in the current implementation.

## Context

The Windows companion and ESP32-C3 need a local bidirectional channel for periodic telemetry, discovery, mode changes, and refresh requests.

## Decision

Use USB CDC serial at 115200 baud with newline-terminated printable ASCII records, a firmware handshake, and a 160-byte complete-line bound.

Both sides reject malformed input rather than normalize it. The companion strictly decodes inbound bytes as ASCII and recognizes a fixed command set. The firmware counts CR bytes toward the pre-LF bound and discards an overflowing or invalid line through its newline.

## Evidence and consequences

The interface is implemented in `quark.py` and `QUARK.ino`; packet format and bound have unit-test coverage. USB avoids introducing a network service on the device. It requires a serial driver/port and allows a local process with port access to send commands. Repository history does not establish the original decision rationale; this record documents the current architecture.
