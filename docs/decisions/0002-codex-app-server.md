# ADR 0002: Read Codex data through the local App Server

## Status

Accepted in the current implementation.

## Context

Codex usage summaries are not collected by the Windows performance library. The companion needs a source associated with the same installed and signed-in Codex CLI used by the user.

## Decision

Start `codex app-server` as a child process and use its JSON-RPC messages over standard input/output to retrieve usage and rate-limit data.

## Evidence and consequences

The implementation is in `quark.py:CodexAppServer`. This reuses the local CLI environment rather than storing a QUARK API key. It depends on the App Server interface, CLI installation, and sign-in state, and the CLI may communicate with OpenAI services. No separate authentication system or network endpoint is implemented by QUARK. Repository history does not establish the original decision rationale.
