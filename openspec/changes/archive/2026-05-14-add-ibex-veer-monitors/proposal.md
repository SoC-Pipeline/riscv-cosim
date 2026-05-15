## Why

The project already has a useful monitor-based retire path for PicoRV32, but
Ibex and VeeR EL2 still use target-specific cosim surfaces. That makes logs,
debug workflows, and future monitor extensions inconsistent across CPU targets.

## What Changes

- Add a project-local monitor path for Ibex that runs in parallel with the
  existing checker and preserves current upstream-style retire, interrupt,
  debug, CSR, and dside synchronization behavior.
- Add a project-local monitor path for VeeR EL2 so retire events flow through
  the same monitor transaction/logging layer as PicoRV32.
- Extend the monitor runtime to support target-specific log naming and reuse
  outside the PicoRV32 VPI wrapper.
- Stage VeeR EL2 monitor support so trace-based retire reporting lands first,
  with follow-up hierarchical GPR and CSR coverage tracked in the same change.
- Document the resulting monitor coverage and target differences.

## Capabilities

### New Capabilities
- `cpu-monitor-integration`: Unified monitor transaction and logging support
  across PicoRV32, Ibex, and VeeR EL2 CPU-mode targets.
- `veer-hierarchical-monitoring`: Hierarchical monitor signal collection plan
  for VeeR EL2 GPR/CSR visibility beyond the public trace port.

### Modified Capabilities
- `none`

## Impact

- Affected code:
  - `src/mon/mon_instr/*`
  - `src/cosim/spike_dpi.cc`
  - `src/top_cpu/tb_ibex*`
  - `src/top_cpu/tb_veer_el2*`
  - `build.sh`, docs, and monitor log outputs as needed
- Affected systems:
  - CPU-mode cosim for Ibex and VeeR EL2
  - Target-specific regression logs and debug evidence
