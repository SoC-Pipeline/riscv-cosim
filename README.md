# RISC-V Co-Simulation

This repository builds RISC-V firmware images and runs RTL co-simulation
targets. PicoRV32 uses Icarus Verilog with the project VPI bridge. Ibex uses a
project-owned Verilator `tb_ibex` top with the upstream Ibex DPI co-simulation
checker.

## Layout

```text
src/top/       Project-owned Verilog/SystemVerilog testbench entry points
src/cosim/     Project VPI bridge C++ sources and Spike backend
firmware/      Firmware tests and linker scripts
scripts/       Python log and hex utilities
external/      External dependencies such as PicoRV32, Ibex, Spike, and pk
build/         Generated firmware, dependency, simulation, and VPI artifacts
docs/          Architecture notes
```

## Tools

Load the expected toolchain modules before building:

```bash
module load riscv-toolchain/master-v20251230 openEDA/verilator/v5.010
```

The build uses `riscv32-unknown-elf-*`, `iverilog`, and `vvp` from the loaded
tool environments. Spike is built from the Ibex-compatible source under
`external/ibex/external/riscv-isa-sim` into `build/spike`; pk is built from
`external/riscv-pk` into `build/pk`.

The Ibex target uses Verilator, FuseSoC, and the local Spike pkg-config
packages from `build/spike`. The expected module set for Ibex is:

```bash
module load riscv-toolchain/master-v20251230 openEDA/verilator/v5.010
```

`./build.sh build -t ibex` and `./build.sh run ibex` check for Verilator
`5.010` before starting the Ibex flow. Set `IBEX_VERILATOR_VERSION=any` only
when intentionally testing another Verilator version.

PicoRV32 and Ibex use the same firmware ELF by default:
`build/firmware/<test>/obj/firmware.elf`. The shared reset vector is
`RESET_VECTOR`, defaulting to `0x80000080`. PicoRV32 receives that value as
`PROGADDR_RESET`; Ibex derives `BootAddr` from the same value because the core
fetches from `{boot_addr_i[31:8], 8'h80}` after reset.

## Run

```bash
./build.sh all
```

Build slow local dependencies once with:

```bash
./build.sh build spike
```

Build firmware or simulation tops explicitly with:

```bash
./build.sh build -f all
./build.sh build -t picorv32
./build.sh build -t ibex
```

Run a specific simulation target and firmware case with:

```bash
./build.sh run picorv32 hello
./build.sh run ibex pico_test
```

The default test is `hello`. Override it with `TEST_NAME=<name>` for a specific
firmware case, or set `FIRMWARE_CASES` to choose the case list used by
`./build.sh all` and `./build.sh run all all`.

For PicoRV32 bridge mode selection, set `PICORV32_COSIM_IF`. The default is
`vpi`. The current Icarus (`iverilog`/`vvp`) flow does not support the project
`dpi` prototype mode, so `PICORV32_COSIM_IF=dpi` exits with a clear error and
keeps `vpi` as the supported runtime path.

Default Spike commit logs are emitted per target:

```text
dump/picorv32_spike_commit.log
dump/ibex_spike_commit.log
dump/veer_el2_spike_commit.log
```

Project compare logs remain wrapper-specific:

```text
dump/picorv32_cosim_result.log
dump/veer_el2_cosim_result.log
```

`env.sh` has been removed. Override build settings with shell environment
variables before invoking `build.sh`; the script resolves and exports defaults
internally for its subcommands:

```bash
export TEST_NAME=hello
export BUILD_JOBS=8
export RESET_VECTOR=0x80000080
./build.sh run ibex hello
```

`./build.sh clean` removes generated firmware, top, cosim, cache, dump, and log
outputs while preserving `build/spike`, `build/spike-build`, `build/pk`, and
`build/pk-build`. Use `./build.sh clean-all` to remove the whole `build/`
directory plus generated dump/log outputs.

Generated files are written under:

```text
build/firmware/<test>/obj/
build/spike/
build/pk/
build/src/cosim/libspike.so
build/src/cosim/libspike.vpi
build/src/top/tb_picorv32.vvp
build/src/top/ibex/
dump/
```

## Documentation

See `docs/arch.md` for the build flow and path ownership model.
