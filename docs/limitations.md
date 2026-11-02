# Limitations

- The companion supports Windows only. It uses Windows startup, mutex, and Codex executable discovery behavior.
- Codex data depends on the installed Codex CLI, its App Server protocol, sign-in state, and returned usage fields. These can change outside this repository.
- The `chatgpt.com` indicator is only a TCP/443 reachability probe; it is not a Codex connection or authentication check.
- GPU readings currently depend on NVIDIA's `nvidia-smi`; other GPU vendors are not supported by this collector.
- CPU temperature is not provided because the current Windows `psutil` path does not report it reliably.
- The OLED is monochrome and small; the UI divides information across pages rather than presenting every metric at once.
- The PCB targets a SuperMini form factor whose clone dimensions can vary. A 3D/physical fit check is still needed before production.
- ERC/DRC reports exclude specified checks. See [verification.md](verification.md); no manufacturing certification is claimed.
- Hardware-in-the-loop, electrical, acoustic, power, thermal, EMC, endurance, and user-study results are absent from this audit.
- The repository has no license file. Public visibility alone does not specify reuse or redistribution terms.
- The source reports version 1.2.0 while the latest published release is v1.1.0; there is no v1.2.0 release artifact from this audit.
