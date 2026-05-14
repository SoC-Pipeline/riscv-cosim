## Context

Target mode:

```text
simulator/Spike executes instructions
  -> drives SoC bus transactions
  -> receives read data / ready / MMIO effects
```

This differs from current CPU-mode retire comparison and requires a separate
SoC-oriented top.

## Design Decisions

### 1) New SoC directory and phase split

- Add SoC-mode sources under `src/top_soc`.
- Keep `src/top_cpu` untouched for existing flows.
- Implement in phases:
  1. RAM read/write only (no internal CPU)
  2. Hook simulator bus driver
  3. Add UART/MMIO/finish and full hello flow

### 2) PicoSoC shell instead of `bind` replacement

`bind` cannot replace an already instantiated CPU. We therefore create a new
top-level shell module that preserves PicoSoC-style decode and memory/peripheral
wiring but exposes an external master interface in place of `picorv32` signals.

### 3) First executable milestone

Provide a testbench that directly drives the external master interface and
verifies memory write/read correctness. This de-risks the SoC shell before
integrating simulator execution.

## Risks

- Address-map mismatches between shell and future simulator integration.
- Timing assumptions on ready/response handshake.

Mitigation:
- Start with synchronous single-beat memory transactions and deterministic
  checks in TB.
