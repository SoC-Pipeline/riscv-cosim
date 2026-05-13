## Why

The repository currently mixes RTL, firmware tests, VPI bridge sources, Python utilities, and external dependency drops in legacy top-level directories. The layout needs to match the documented project structure while preserving the existing PicoRV32/Spike co-simulation regression.

## What Changes

- Move RTL sources from `top/` to `src/top/`.
- Move firmware test inputs from `tests/` to `firmware/`.
- Move C/C++ co-simulation bridge headers and sources from `scripts/` to `src/`, while keeping Python utilities in `scripts/`.
- Keep external dependencies under `external/`, including `external/riscv-isa-sim` and `external/picorv32`.
- Update build scripts, environment setup, hard-coded testbench defaults, and clean targets to use the new paths.
- Add architecture documentation at `docs/arch.md`.
- Preserve `make all` as the primary regression entry point.

## Capabilities

### New Capabilities
- `cosim-layout`: Defines the repository layout and build-path behavior for the PicoRV32/Spike co-simulation flow.

### Modified Capabilities

## Impact

- Affects `Makefile`, `env.sh`, `README.md`, `docs/arch.md`, Verilog testbench defaults, and file locations under `src/`, `firmware/`, and `scripts/`.
- The main regression remains `make all` after loading `riscv-toolchain/master-v20251230` and `openEDA/verilator/v5.046`.
- No runtime behavior change is intended beyond path synchronization.
