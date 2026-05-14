# Design: PicoSoC CPU Slot Replacement with Spike Bus Master

## Summary

Move to an in-SoC replacement model:
- Preserve PicoSoC structure and device map from upstream `picosoc.v`.
- Replace only CPU instance with `spike_bus_master`.
- Drive Spike through DPI from module-level logic, not testbench-level C++ top.

## Source Layout

- Migrated minimal vendor source:
  - `src/top_soc/picosoc.v` (from `external/picorv32/picosoc/picosoc.v`)
- New project-owned sources:
  - `src/top_soc/spike_bus_master.sv`
  - `src/top_soc/spike_bus_dpi.cc`
  - `src/top_soc/tb_picosoc_soc.sv` (SoC TB: preload/reset/finish/timeout)

Unmigrated upstream dependencies remain referenced from `external/picorv32` as needed.

## Interface Contract

`spike_bus_master` exports PicoRV32-like memory bus signals:
- request: `mem_valid`, `mem_instr`, `mem_addr`, `mem_wdata`, `mem_wstrb`
- response: `mem_ready`, `mem_rdata`

DPI backend keeps Spike state in a worker thread and converts Spike
`simif_t` fetch/load/store callbacks into PicoSoC bus requests. The
SystemVerilog wrapper returns responses only for active `mem_valid &&
mem_ready` handshakes.

## Firmware Image Handling

Build firmware as ELF and HEX.

Preferred HEX path:
- `riscv64-unknown-elf-elf2hex` (if available)

Fallback:
- existing objcopy/makehex path with explicit conversion constraints.

TB uses HEX preload into SoC RAM (`$readmemh`) and reset release flow.

## Data Flow

1. Build firmware (`.elf` + `.hex`).
2. TB preloads RAM with `.hex`.
3. Reset releases.
4. `spike_bus_master` issues bus requests via SoC interconnect.
5. SoC RAM/MMIO returns responses.
6. Firmware writes finish marker and simulation exits.

## Validation

- `./build.sh run soc picorv32 hello`
- `./build.sh run soc picorv32 all`
- `./build.sh run cpu all hello`
