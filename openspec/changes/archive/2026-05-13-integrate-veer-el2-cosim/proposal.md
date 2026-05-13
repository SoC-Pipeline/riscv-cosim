## Why

The project currently supports PicoRV32 and Ibex cosimulation, but TODO.md now requires VeeR EL2 to be usable through the same build and firmware workflow. Integrating VeeR EL2 expands the cosim environment from single-core examples into a reusable multi-core harness model.

## What Changes

- Add a project-owned VeeR EL2 top under `src/top/tb_veer_el2.sv`.
- Add `veer_el2` as a build/run target in `build.sh`.
- Reuse firmware cases from `firmware/` and the shared `RESET_VECTOR` convention.
- Connect VeeR EL2 retire trace events to the project Spike cosim path.
- Keep VeeR EL2 generated configuration and build outputs under `build/src/top/veer_el2`.
- Update architecture documentation for the new target and integration model.

## Capabilities

### New Capabilities

- `veer-el2-cosim-target`: VeeR EL2 can be built and run from the project build script with project firmware and Spike cosimulation.

### Modified Capabilities

- `build-flow`: The build flow accepts `veer_el2` as a top/run target without changing existing PicoRV32 and Ibex behavior.

## Impact

- `src/top/tb_veer_el2.sv`
- `build.sh`
- `docs/arch.md`
- `openspec/changes/integrate-veer-el2-cosim`
- VeeR EL2 generated files under `build/src/top/veer_el2`
