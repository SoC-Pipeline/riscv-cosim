## Why

PicoRV32 CPU-mode co-simulation still depends on the `iverilog`/`vvp` + VPI flow, while the other CPU and SoC paths have already moved to Verilator + DPI. Keeping a separate VPI-only path increases maintenance cost, complicates build logic, and blocks reuse of the newer co-simulation runtime pattern.

## What Changes

- Replace the PicoRV32 CPU-mode top build/run flow with a Verilator-based executable that keeps `tb_picorv32` as the simulation top.
- Remove PicoRV32's dependence on `$cosim_*` VPI tasks and replace them with DPI-C imports and a dedicated C++ cosim harness.
- Keep the existing PicoRV32 retire-monitor semantics, including full RVFI retire payload forwarding into `mon_instr`.
- Update the PicoRV32 build/run path in `build.sh` so CPU-mode simulation no longer requires `libspike.vpi` or `vvp`.

## Capabilities

### New Capabilities
- `picorv32-cosim-verilator`: Defines the PicoRV32 CPU-mode simulation behavior when running under Verilator with DPI-backed co-simulation.

### Modified Capabilities

## Impact

- Affected code: `src/top_cpu/tb_picorv32.v`, new PicoRV32 Verilator cosim C++ harness, shared cosim helper usage, and `build.sh`.
- Removed dependency path: PicoRV32 CPU-mode VPI task registration and `vvp` runtime usage.
- Runtime behavior: PicoRV32 CPU-mode simulation remains top-level testbench driven, but is executed as a Verilator binary with DPI callbacks.
