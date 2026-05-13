## Why

The active co-simulation flow is still shaped around one PicoRV32 testbench and
Spike-specific Verilog tasks. A design that already has its own testbench should
not need to copy the current scoreboard arrays, golden stepping loop, or
PicoRV32-specific hierarchy references. The co-simulation layer should be
usable as a small plugin that a design testbench calls at instruction retire
events.

## What Changes

- Replace the active Verilog-facing task API with simulator-neutral `$cosim_*`
  tasks.
- Add a C++ co-simulation session that owns compare counters, logging, and
  reference simulator stepping.
- Move PC/instruction comparison out of the PicoRV32 Verilog testbench and into
  the C++ VPI runtime.
- Update the current PicoRV32 example testbench to call `$cosim_init`,
  `$cosim_retire`, and `$cosim_finish`.
- Do not preserve `$spike_*` compatibility tasks in this change.

## Capabilities

### New Capabilities

- `pluggable-cosim-testbench-api`: Defines the neutral `$cosim_*` task API that
  a design-owned testbench can call.

### Modified Capabilities

None.

## Impact

- Affects active C++ VPI files under `src/cosim`.
- May add a C++ session class under `src/cosim`.
- Affects `src/top/tb_picorv32.v` as the PicoRV32 example integration.
- Affects `build.sh` if new C++ source files are added.
- Affects `docs/arch.md` to describe the plugin-style interaction.
