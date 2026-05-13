## Why

The project currently has a PicoRV32 co-simulation path, but TODO.md calls for
bringing in the Ibex Verilator co-simulation environment as a separate target.
Ibex already ships a lockstep cosim checker with richer RVFI, CSR, interrupt,
and memory-access checks, so the first step should integrate that environment
without forcing it into the PicoRV32 `$cosim_*` VPI shape.

## What Changes

- Add an Ibex simulation target reachable through `./build.sh sim ibex`.
- Keep the existing PicoRV32 Icarus/VPI flow as the default simulation target.
- Build an Ibex-specific firmware image from the existing firmware test sources,
  linked for the Ibex simple-system memory map and halt protocol.
- Use Verilator and the Ibex simple-system cosim environment for the Ibex target.
- Keep Ibex cosim integration separate first; broader cosim abstraction can
  happen after the Ibex path is running.
- Update documentation to describe both simulation targets and Ibex-specific
  runtime requirements.

## Capabilities

### New Capabilities

- `ibex-cosim-target`: Adds a runnable Ibex Verilator co-simulation target using
  the Ibex simple-system cosim environment and project firmware inputs.

### Modified Capabilities

- `build-entrypoints`: Extends the build script simulation entry point so users
  can select `picorv32` or `ibex` targets while preserving current defaults.

## Impact

- Affects `build.sh`, `src/top/tb_ibx.sv`, firmware build inputs or linker
  support, and `docs/arch.md`.
- Depends on `external/ibex` and its FuseSoC/Verilator simple-system cosim
  flow.
- Requires an Ibex-compatible Spike installation with pkg-config metadata for
  `riscv-riscv`, `riscv-disasm`, and `riscv-fdt`.
- Does not remove or refactor the active PicoRV32 cosim implementation.
