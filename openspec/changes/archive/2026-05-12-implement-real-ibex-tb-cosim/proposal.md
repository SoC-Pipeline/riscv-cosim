## Why

The Ibex target currently runs through the upstream `ibex_simple_system` top,
while `src/top/tb_ibex.sv` is only a marker. TODO.md now calls for a real
project-owned Ibex testbench that can run cosim and share the same firmware ELF
as the PicoRV32 case.

## What Changes

- Replace the marker `src/top/tb_ibex.sv` with a real Ibex Verilator testbench
  derived from the upstream simple-system structure.
- Make the Ibex reset vector configurable and default it to the PicoRV32
  firmware base so both targets can use the same `firmware.elf`.
- Add project-owned Ibex cosim binding and Verilator C++ harness files for the
  `tb_ibex` top instead of relying on upstream `ibex_simple_system` naming.
- Update `./build.sh sim ibex` to build and run the project `tb_ibex` cosim
  target with the shared firmware ELF.
- Remove the Ibex-only firmware variant from the active build path when it is
  no longer needed.
- Update architecture documentation for the project-owned Ibex testbench.

## Capabilities

### New Capabilities

- `real-ibex-tb-cosim`: Project-owned Ibex Verilator cosim testbench with
  configurable reset vector and shared firmware ELF support.

### Modified Capabilities

- `build-entrypoints`: The Ibex simulation target now builds and runs the
  project `tb_ibex` cosim top and uses the same firmware ELF as PicoRV32.

## Impact

- Affects `src/top/tb_ibex.sv`, new Ibex cosim support files under `src/top`,
  `build.sh`, `docs/arch.md`, and README/TODO notes as needed.
- Depends on `external/ibex` FuseSoC cores, Verilator 5.010, and the Ibex cosim
  Spike pkg-config packages.
- Keeps PicoRV32 behavior and its default simulation target unchanged.
