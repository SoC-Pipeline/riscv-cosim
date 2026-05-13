## Context

All active cosim targets use a local Spike build installed under `build/spike`,
but the wrappers around Spike differ:

- PicoRV32 uses the project VPI frontend plus `CosimSession` and
  `SpikeSimulator`.
- VeeR EL2 uses a project Verilator DPI frontend plus the same
  `CosimSession` and `SpikeSimulator`.
- Ibex uses the upstream Ibex `SpikeCosim` checker.

Today, Ibex and VeeR EL2 configure default Spike commit logs, while PicoRV32
does not. PicoRV32 also keeps stale fallback paths to the removed
`arith_basic_test` default. This causes two practical problems: logs are not
generated uniformly, and existing files can be misread as cross-core mismatches
when they were produced from different firmware cases.

## Goals / Non-Goals

**Goals:**
- Make all three active cosim targets emit a Spike commit log by default.
- Use explicit per-target default log paths so output ownership is obvious.
- Align PicoRV32 fallback firmware paths with the current `hello` default.
- Keep the existing wrapper split intact while documenting it accurately.

**Non-Goals:**
- Replacing the Ibex upstream `SpikeCosim` path with `CosimSession`
- Forcing all future trap/debug/interrupt-heavy scenarios to produce identical
  commit logs
- Refactoring unrelated build or testbench structure

## Decisions

1. Add an explicit PicoRV32 Spike commit log environment variable in `build.sh`.

   This matches the existing Ibex and VeeR EL2 model and makes PicoRV32's
   default logging behavior visible from the build interface.

2. Pass PicoRV32 runtime configuration through environment variables read by
   `src/cosim/spike_dpi.cc`.

   The VPI path already resolves the ELF path dynamically. Extending it with
   `PICORV32_*` environment variables keeps the change local and avoids adding
   new Verilog-facing task arguments.

3. Keep log equivalence scoped to shared bare-metal firmware flows.

   `hello` and `pico_test` use a shared reset vector and MMIO contract, so they
   are the correct baseline for comparing commit logs. Ibex still uses a richer
   upstream checker and remains structurally different by design.

4. Preserve the current compare log naming split.

   `CosimSession` compare logs remain per-wrapper outputs. This change only
   standardizes the default Spike commit logs and the default firmware/log
   wiring.

## Risks / Trade-offs

- [Different wrappers still model more than `(pc, instr)` differently] ->
  Document that commit-log identity is expected for shared simple firmware, not
  for all future scenarios.
- [New default PicoRV32 log files may affect local scripts that assumed no log]
  -> Use an explicit file path and describe it in help/docs.
- [Stale dump files can still confuse manual comparisons] -> Use per-target log
  names and update docs to explain what each file represents.
