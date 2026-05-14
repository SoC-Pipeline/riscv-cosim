# RISC-V Co-Simulation

This repository builds firmware and runs RISC-V co-simulation flows for:
- CPU retire-compare mode: `picorv32`, `ibex`, `veer_el2`
- SoC bus-driven mode (CPU-less shell): `soc/picorv32`

## Repository Overview

```text
                 +-----------------------------+
                 |        firmware/*           |
                 |  hello / pico_test / mem    |
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
| CPU Cosim Targets              |    | SoC Mode (CPU-less)      |
| picorv32 / ibex / veer_el2     |    | picosoc soc + Spike bus  |
+---------+----------------------+    +------------+-------------+
          |                                        |
          v                                        v
+---------+----------------------+    +------------+-------------+
| src/top_cpu + src/cosim        |    | src/top_soc               |
| retire events <-> CosimSession |    | ELF preload -> RAM bus RW |
+---------+----------------------+    +------------+-------------+
          |                                        |
          +-------------------+--------------------+
                              v
                    +---------+---------+
                    |      log/ dump/   |
                    | run logs + commits|
                    +-------------------+
```

## Prerequisites

```bash
module load riscv-toolchain/master-v20251230 openEDA/verilator/v5.010
```

Initialize submodules on fresh checkout:

```bash
git submodule update --init --recursive
```

## Quick Start

Run full CPU-target regression:

```bash
./build.sh run cpu all all
```

Build slow dependencies once:

```bash
./build.sh build spike
```

Build firmware/top explicitly:

```bash
./build.sh build firmware all
./build.sh build cpu all
./build.sh build soc picorv32
```

Run one target/case:

```bash
./build.sh run cpu picorv32 hello
./build.sh run cpu ibex pico_test
./build.sh run cpu veer_el2 mem
```

Run SoC bus-driven mode:

```bash
./build.sh run soc picorv32 hello
./build.sh run soc picorv32 all
```

## Common Environment Overrides

```bash
export BUILD_JOBS=8
export RESET_VECTOR=0x80000080
export FIRMWARE_CASES="hello pico_test mem"
```

## Output Locations

Main generated artifacts:

```text
build/firmware/<test>/obj/
build/spike/
build/pk/
build/src/top_cpu/ibex/
build/src/top_cpu/veer_el2/
build/src/top_cpu/soc/
log/
dump/
```

`./build.sh clean` keeps spike/pk build caches.
`./build.sh clean-all` removes the whole `build/` directory.

## More Details

See `docs/arch.md` for architecture and layer-level design details.
