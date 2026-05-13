## Context

The previous simulator-interface refactor isolated Spike behind
`RiscvSimulator`, but the active Verilog interaction still requires a testbench
to manually pull reference PC/instruction values, step the simulator, store both
golden and DUT streams, and run the final compare. That makes cosim hard to
reuse across designs with their own testbenches.

## Goals

- Make the co-simulation layer callable as a small testbench plugin.
- Use simulator-neutral `$cosim_*` task names.
- Let each design provide a retire event with DUT PC and instruction.
- Keep compare counters and log generation inside C++ cosim runtime.
- Keep Spike as the default backend through the existing `RiscvSimulator`
  abstraction.
- Keep the current PicoRV32 regression behavior and zero-failure result.

## Non-Goals

- Do not keep `$spike_*` compatibility tasks.
- Do not add a second simulator backend in this change.
- Do not require a standardized DUT interface such as RVFI yet.
- Do not rewrite the AXI memory model or firmware flow.
- Do not make every design use the current PicoRV32 testbench.

## Design

Add a `CosimSession` C++ class:

```text
CosimSession
  init(config)
  retire(dut_pc, dut_instr)
  finish()
  pass_count()
  fail_count()
```

`CosimSession` owns one `RiscvSimulator` backend, currently a
`SpikeSimulator`. On each `retire` call it:

1. Reads the reference PC.
2. Fetches the reference instruction at that PC.
3. Skips reference entries outside the configured memory range or with invalid
   instruction fetch result, matching the existing filtering behavior.
4. Compares reference PC/instruction against the DUT retire PC/instruction.
5. Logs the compare row.
6. Steps the reference simulator by one instruction when the reference
   instruction is not an ebreak instruction.

The active VPI frontend registers:

```text
$cosim_init(elf_path)
$cosim_retire(dut_pc, dut_instr)
$cosim_finish()
$cosim_get_status(pass_count, fail_count)
```

The current PicoRV32 testbench becomes an example integration. It keeps its
existing way of detecting a retire event through PicoRV32 internal debug
signals, but it no longer owns golden arrays or final compare loops. At reset
release it calls `$cosim_init`. On each valid retire it calls
`$cosim_retire(dut_pc, dut_instr)`. On trap it records the final retire sample,
calls `$cosim_finish`, and exits.

The VPI module file can remain `libspike.so`/`libspike.vpi` as a build artifact
name for now, but the Verilog task API becomes `$cosim_*`.

## Future Direction

- Add `+COSIM_BACKEND=<name>` once a second backend exists.
- Add a reusable SystemVerilog include or adapter module for RVFI.
- Split PicoRV32-specific retire tapping into a separate adapter module.
- Rename the build artifact from `libspike` to `libcosim` in a later change if
  needed.

## Risks

- Moving compare timing from Verilog arrays to immediate C++ retire comparison
  can expose off-by-one behavior around the final trap sample.
- The current PicoRV32 retire signal is internal and not a generic retire
  contract; this change should only make it an example integration.
- The log format should remain close enough for existing debugging.

## Verification

- Run `bash -n build.sh`.
- Run `./build.sh vpi` with required modules loaded.
- Run `./build.sh sim`.
- Run `./build.sh clean && ./build.sh all`.
- Confirm `COMPARE PASS NUM = 11416, COMPARE FAIL NUM = 0`.
