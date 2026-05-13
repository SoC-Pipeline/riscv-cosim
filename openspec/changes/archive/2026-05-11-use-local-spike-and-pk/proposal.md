## Why

The active co-simulation still links Spike and finds `pk` from the loaded toolchain installation. `TODO.md` calls for using repository-local Spike and pk dependencies, with generated dependency builds under `build/`, so the project has explicit dependency provenance.

## What Changes

- Build Spike from `external/riscv-isa-sim` into `build/spike`.
- Build pk from `external/riscv-pk` into `build/pk`.
- Link the VPI bridge against the locally installed Spike headers and libraries.
- Run simulation with the locally installed pk binary by default.
- Keep the RISC-V toolchain path only for cross compiler/binutils lookup.
- Update `.gitmodules`, documentation, and `TODO.md` to describe the local Spike/pk dependency flow.

## Capabilities

### New Capabilities

- `local-spike-pk-dependencies`: Defines local Spike and pk source dependencies and their generated build/install locations.

### Modified Capabilities

None.

## Impact

- Affects `build.sh`, `.gitmodules`, `README.md`, `docs/arch.md`, and `TODO.md`.
- Uses `external/riscv-isa-sim` and `external/riscv-pk` as source dependency directories.
- Adds generated dependency outputs under `build/spike-build`, `build/spike`, `build/pk-build`, and `build/pk`.
- Preserves the active firmware, RTL, VPI, and PicoRV32/Spike comparison behavior.
