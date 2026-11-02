# Hardware workflow research notes

The repository's KiCad source was reviewed against the five skill repositories named in the audit prompt. Crawl4AI 0.9.5 was used in a temporary environment to fetch available repository documentation on 2026-11-02. These references informed a checklist; their code and automation were not installed or copied into QUARK. See the [research source ledger](research-sources.md) for retrieval limits and official documentation.

| Source | Relevant practice | Application and limit |
|---|---|---|
| [kicad-happy](https://github.com/aklofas/kicad-happy) | Agent-assisted KiCad workflow | Kept design decisions grounded in the source schematic/PCB and native KiCad reports. |
| [AI-PCB-layout](https://github.com/ChengminLi/AI-PCB-layout) | Derive power-electronics layout rules from current, switching loops, insulation, thermal needs, and measurements. | Its high-power switching analysis is not directly applicable to this USB-powered, low-speed carrier. No thermal, EMC, or parasitic analysis was claimed. |
| [kicad-mcp-pro](https://github.com/oaslananka/kicad-mcp-pro) | ERC/DRC, DFM and manufacturing review should be distinct evidence categories. | Re-ran CLI ERC and DRC; the reports' ignored checks are documented. This is not a complete fabrication/assembly certification. |
| [KiStack](https://github.com/BackplaneWorks/kistack) | Review schematic, PCB, BOM, and manufacturing outputs in stages. | The prompt URL redirects to this canonical repository. Existing BOM, Gerbers, drills, and source were inventoried; no new fabrication outputs were generated. |
| [circuit-skills](https://github.com/punkfab/circuit-skills) | Hardware-specific review can complement CAD rule checks. | Used only as a review reference; no external skill was installed and no unmeasured electrical claim was added. |

The native KiCad CLI output remains the evidence for the reported ERC/DRC result. The exact included and ignored checks are in [verification.md](verification.md).
