# RISC-V Co-Simulation

This repository builds small RISC-V firmware cases and runs two validation flows:

- CPU retire-compare mode for `picorv32`, `ibex`, and `veer_el2`
- SoC bus-driven mode for `soc/picorv32`, where Spike replaces the CPU slot and accesses PicoSoC devices through the SoC bus

## Repository Layout

```text
build.sh        Build/run entry point. Use ./build.sh help for the command matrix.
src/top_cpu/    CPU-mode testbenches and target-specific cosim adapters.
src/top_soc/    PicoSoC-based SoC mode, Spike bus master, and SoC testbench.
src/cosim/      Shared C++ cosim bridge, CosimSession, and Spike backend.
firmware/       Self-checking firmware cases plus shared startup/runtime code.
scripts/        Build-time helper scripts.
external/       Vendor dependencies: PicoRV32, Ibex, Spike, pk, VeeR EL2.
build/          Generated firmware, dependency builds, and simulator outputs.
log/            Run logs, compare logs, and Spike commit logs.
docs/arch.md    Architecture, data flow, firmware loading, and protocol details.
```

```text
                 +-----------------------------+
                 |        firmware/*           |
                 | hello / pico_test / mem     |
                 +--------------+--------------+
                                |
                                v
                      +---------+---------+
                      |       build.sh    |
                      | build / run / all |
                      +----+---------+----+
                           |         |
          +----------------+         +----------------+
          v                                       v
+---------+----------------------+    +------------+-------------+
| CPU Cosim Targets              |    | SoC Mode                  |
| picorv32 / ibex / veer_el2     |    | PicoSoC + Spike bus      |
+---------+----------------------+    +------------+-------------+
          |                                        |
          v                                        v
+---------+----------------------+    +------------+-------------+
| retire trace -> CosimSession   |    | HEX preload -> SoC RAM   |
| CosimSession -> Spike backend  |    | Spike fetch/load/store   |
+---------+----------------------+    +------------+-------------+
          |                                        |
          +-------------------+--------------------+
                              v
                    +---------+---------+
                    |        log/       |
                    | run logs + commits|
                    +-------------------+
```

## Prerequisites

The common development environment uses a RISC-V GCC toolchain, `elf2hex`, Icarus Verilog, Verilator, FuseSoC, and the external submodules.

```bash
module load riscv-toolchain/master-v20251230 openEDA/verilator/v5.010
git submodule update --init --recursive
```

## Usage

`build.sh` is the command source of truth. Start with:

```bash
./build.sh help
```

Common smoke run:

```bash
./build.sh all
```

Typical focused runs:

```bash
./build.sh run cpu picorv32 hello
./build.sh run soc picorv32 all
```

Common environment overrides are also listed by `./build.sh help`. Frequently used ones include:

```bash
export BUILD_JOBS=8
export RESET_VECTOR=0x80000080
export FIRMWARE_CASES="hello pico_test mem"
```

`hello`, `pico_test`, and `mem` remain the shared supported cases for `./build.sh all`.

## Outputs

Generated artifacts live under `build/`; logs live under `log/`.

Important output areas:

```text
build/firmware/<case>/obj/      Firmware ELF/HEX artifacts.
build/spike/                    Local Spike install.
build/pk/                       Local proxy kernel install.
build/src/top_cpu/              CPU-mode simulator outputs.
build/src/top_cpu/soc/          SoC-mode Verilator output.
log/                            Run logs, compare logs, commit logs.
```

Use `./build.sh clean` for normal generated-output cleanup while preserving slow dependency builds. Use `./build.sh clean-all` to remove the full `build/` tree and logs.

## More Details

See [docs/arch.md](docs/arch.md) for architecture, CPU/SoC data flows, firmware HEX loading, and the firmware/TB protocol.

## References

1. [Ibex co-simulation documentation](https://ibex-core.readthedocs.io/en/latest/03_reference/cosim.html)
1. [spike-cosim](https://github.com/farukyld/spike-cosim)
1. [cosim-arch-checker](https://github.com/tenstorrent/cosim-arch-checker)
