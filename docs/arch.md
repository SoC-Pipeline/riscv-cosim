# Architecture

## Directory Roles

```text
src/top_cpu/   CPU-mode Verilog/SystemVerilog testbench entry points
src/top_soc/   SoC-mode sources
src/cosim/     C++ bridge frontend, simulator interface, and Spike backend
firmware/      Firmware test inputs, startup code, and linker scripts
scripts/       Python utilities used by firmware and log processing flows
external/      External dependencies such as PicoRV32, Ibex, Spike, and pk
build/         Generated firmware, dependency, simulation, and VPI artifacts
log/           Generated run logs, compare logs, and Spike commit logs
```

`src/` contains project-owned source code. Python helper programs stay in
`scripts/` because they are invoked as build utilities rather than compiled
source.

## Main Flow

The primary regression entry point is:

```bash
./build.sh all
```

It ensures missing artifacts are built, then runs every active firmware case on
all CPU-mode targets (`picorv32`, `ibex`, `veer_el2`) and the active SoC-mode
target (`soc/picorv32`). The build script separates dependency, firmware, top,
and execution layers:

```text
./build.sh build spike
  -> build external/ibex/external/riscv-isa-sim into build/spike
  -> build external/riscv-pk into build/pk
  -> build build/src/cosim/libspike.so and libspike.vpi

./build.sh build firmware [case|all]
  -> build build/firmware/<case>/obj/firmware.elf
  -> build build/firmware/<case>/obj/firmware.hex

./build.sh build cpu [picorv32|ibex|veer_el2|all]
  -> build build/src/top_cpu/tb_picorv32.vvp
  -> build build/src/top_cpu/ibex/.../Vtb_ibex
  -> build build/src/top_cpu/veer_el2/obj_dir/Vtb_top

./build.sh build soc [picorv32|all]
  -> build build/src/top_cpu/soc/obj_dir/Vtb_picosoc_soc

./build.sh run cpu [picorv32|ibex|veer_el2|all] [case|all]
  -> ensure required firmware and top artifacts
  -> execute the selected simulation matrix

./build.sh run soc [picorv32|all] [case|all]
  -> ensure required firmware and SoC top artifacts
  -> execute Spike-driven SoC bus runs
```

`./build.sh all` is equivalent to `./build.sh run all all all`.

The PicoRV32 run flow uses the project VPI module:

```text
firmware
  -> link once for the shared PicoRV32/Ibex RAM base at 0x80000000
  -> place the shared reset vector at 0x80000080

picorv32 top
  -> build tb_picorv32.vvp with Icarus Verilog
  -> run vvp with build/src/cosim/libspike.vpi
```

The Ibex run flow uses the upstream Ibex DPI checker:

```text
firmware
  -> reuse the same build/firmware/<case>/obj/firmware.elf

ibex top
  -> build local:spike_cosim:tb_ibex_cosim through FuseSoC
  -> write Verilator outputs under build/src/top_cpu/ibex
  -> run Vtb_ibex --meminit=ram,<shared firmware elf>
  -> use local build/spike pkg-config files for SpikeCosim
```

The VeeR EL2 run flow uses the project `CosimSession` through a Verilator DPI
bridge:

```text
firmware
  -> reuse the same build/firmware/<case>/obj/firmware.elf
  -> emit build/firmware/<case>/obj/firmware_veer.hex for VeeR memory loading

veer_el2 top
  -> generate VeeR configuration under build/src/top_cpu/veer_el2/snapshots/default
  -> build external/Cores-VeeR-EL2/testbench/tb_top.sv with the project monitor
  -> run Vtb_top with +ELF_PATH=<shared firmware elf>
  -> feed VeeR retire trace PC/instruction events into CosimSession
```

## SoC Mode (PicoSoC CPU Slot Replacement)

The repository also includes a bus-driven SoC validation mode where the
simulator replaces the CPU slot inside a project-owned PicoSoC copy:

```bash
./build.sh run soc [picorv32|all] [hello|pico_test|mem|all]
```

This mode is intentionally different from retire-compare CPU cosim:
- CPU mode (`run cpu picorv32|ibex|veer_el2`) checks DUT retire events against Spike.
- SoC mode (`run soc picorv32`) removes DUT CPU from the execution path and lets Spike
  execute firmware while issuing bus transactions into SoC devices.

### SoC Structure

`src/top_soc/picosoc.v` is a minimal migrated copy of upstream PicoSoC. It keeps
the upstream RAM, SPI flash interface, UART registers, and external IO path, but
replaces the original PicoRV32 CPU instance with `spike_bus_master`.

Bus roles:
- Master: Spike through `spike_bus_master.sv` and `spike_bus_dpi.cc`
- Slaves:
  - PicoSoC RAM (`picosoc_mem`)
  - PicoSoC UART/SPI registers
  - TB-handled simulation output and status MMIO

Address map used by this mode:
- `0x80000000` region: RAM
- `0x02000000` region: PicoSoC SPI/UART registers
- `0x10000000`: shared firmware/TB simulation MMIO

The shared simulation MMIO address is project-owned and is decoded explicitly
before the PicoSoC SPI flash path. Firmware writes byte-sized protocol values
and printable characters to `0x10000000`; the SoC TB observes those writes on
`iomem_valid/iomem_wstrb/iomem_addr/iomem_wdata`.

### Firmware Load Path In SoC Mode

SoC mode uses the PicoSoC-style HEX preload path:

```text
firmware.elf
  -> build/firmware/<case>/obj/firmware.hex
  -> TB $readmemh into uut.memory.mem
  -> reset release
  -> set Spike PC to ELF entry
  -> Spike fetch/load/store requests cross the PicoSoC bus
```

The TB is the only firmware image loader. `spike_bus_dpi.cc` does not parse or
preload HEX/ELF contents into private RAM. Its `simif_t` implementation returns
`nullptr` from `addr_to_mem()`, forcing Spike fetches, loads, and stores through
the DPI bus bridge.

### Evidence That Execution Uses SoC RAM

`run soc ...` now prints bus access counters at runtime:

```text
[SOC] ram_reads=<N> ram_writes=<N>
```

These counters are collected from `uut.mem_valid && uut.mem_ready && uut.ram_sel`
inside `tb_picosoc_soc.sv`, so they confirm that Spike executed through the
PicoSoC RAM path rather than through private simulator memory.

### SoC Logs

For each case, `run soc ...` writes:
- run log: `log/run_soc_spike_<case>.log`
- commit log: `log/soc_spike_<case>_commit.log`

## Co-Simulation Split

### Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    Testbench (RTL)                         │
│  picorv32 (VPI)  │  ibex (DPI)  │  veer_el2 (DPI)          │
└────────────┬──────────────┬──────────────┬─────────────────┘
             │              │              │
             ▼              ▼              ▼
      ┌──────────────────────────────────────────┐
      │           cosim_bridge (C ABI)            │
      │  init, retire, step_detail, finish, reset  │
      └────────────────────┬─────────────────────┘
                           │ calls
                           ▼
      ┌──────────────────────────────────────────┐
      │           CosimSession                    │
      │  retire(), step_detail(), set_csr(), etc. │
      └────────────────────┬─────────────────────┘
                           │ delegates
                           ▼
      ┌──────────────────────────────────────────┐
      │      SpikeSimulator (processor_t)        │
      │  step(), pc(), reg(), set_csr(), get_csr()│
      └──────────────────────────────────────────┘
```

**Layer responsibilities:**
- **Bridge (cosim_bridge)**: C ABI wrapper, DPI/VPI shim, no verification logic
- **CosimSession**: Unified session layer, retire comparison, pass/fail counters
- **SpikeSimulator**: Spike backend using `processor_t` over a project `simif_t`
  implementation, plus commit log generation

The active VPI module currently keeps the build artifact name `libspike`, but
the Verilog-facing API is simulator-neutral.

`src/cosim/spike_dpi.cc` includes both VPI and DPI wrappers. The current
PicoRV32 runtime path uses the VPI wrapper, which handles task registration,
plusargs, VPI argument conversion, and result writes back to Verilog.

`src/cosim/cosim_session.cc` owns one reference simulator backend, compare
counters, retire-event comparison, and log generation.
Design-owned testbenches interact with it through:

```text
$cosim_init(elf_path)
$cosim_retire(dut_pc, dut_instr)
$cosim_finish()
$cosim_get_status(pass_count, fail_count)
```

`src/cosim/riscv_simulator.h` defines the neutral reference-simulator operations
used by the current scoreboard: initialize, step, read PC, fetch an instruction,
read a register, CSR sync, interrupt/debug sync, and finished-state query.
It also defines `SimulatorCapabilities`, used by `CosimSession` to gate optional
behavior when a backend does not support all synchronization channels.

`src/cosim/spike_simulator.cc` is the only implemented backend in this change.
It owns Spike configuration, simulator, memory, and processor state, and it uses
the local Spike installation under `build/spike`.

`src/cosim/simulator_factory.cc` maps `CosimConfig.backend` to a backend
implementation. Current default is `backend=spike`.

### Adding A New Simulator Backend

To add a new backend with minimal changes:

1. Implement a new `RiscvSimulator` subclass in `src/cosim/`.
2. Implement `capabilities()` accurately for that backend.
3. Register backend creation in `simulator_factory.cc` using a new backend name.
4. Ensure build scripts include the new backend source files.

Capability guidance:
- `instruction_fetch=false`: `CosimSession::retire()` degrades to non-compare
  pass counting and skips golden instruction matching.
- `csr_access=false`: CSR synchronization APIs are ignored.
- `interrupt_sync=false`: `set_mip`/`set_nmi` sync APIs are ignored.
- `debug_req=false`: debug request sync API is ignored.

This keeps wrappers and `cosim_bridge` stable while allowing backends with
different feature surfaces.

The repository shares one local Spike build and now also shares the project
`CosimSession` execution path across PicoRV32, Ibex, and VeeR EL2.

The PicoRV32 example testbench is `src/top_cpu/tb_picorv32.v`. It keeps its own
clock, reset, memory, and DUT wiring, and calls `$cosim_retire` when it observes
a retired DUT instruction. Other designs can use their own testbench and only
need to provide equivalent retire PC/instruction events.

The VeeR EL2 project monitor is `src/top_cpu/tb_veer_el2.sv`. It follows the same
PicoRV32 cosim contract but uses Verilator DPI functions because VeeR is built
as a Verilator executable. The monitor initializes cosim from the selected ELF,
reports `trace_rv_i_valid_ip`, `trace_rv_i_address_ip`, and
`trace_rv_i_insn_ip`, and finishes cosim when firmware writes the shared finish
value. `src/top_cpu/tb_veer_el2_cosim.cc` adapts those DPI calls to the same
`CosimSession` used by PicoRV32. VeeR cosim writes its compare log to
`log/veer_el2_cosim_result.log` by default and enables Spike commit logging at
`log/veer_el2_spike_commit.log`, matching the Ibex cosim debug flow.

The PicoRV32 VPI frontend now follows the same default logging policy. It writes
its compare log to `log/picorv32_cosim_result.log` and enables Spike commit
logging at `log/picorv32_spike_commit.log` by default.

Across PicoRV32, Ibex, and VeeR EL2, cosim log-path selection now follows one
policy keyed by `cpu_name`:

```text
1) COSIM_LOG_PATH / SPIKE_COMMIT_LOG_PATH
2) target-specific env vars
3) log/<cpu_name>_cosim_result.log
   log/<cpu_name>_spike_commit.log
```

`build.sh` also exposes `PICORV32_COSIM_IF` as a mode selector (`vpi|dpi`) for
bridge-interface experiments. In the current repository flow, PicoRV32 runtime
is supported only with `vpi` under Icarus (`iverilog`/`vvp`). Selecting `dpi`
is treated as an unsupported mode and fails fast with an explicit diagnostic.

The Ibex example target is implemented in project sources by `src/top_cpu/tb_ibex.sv`.
It instantiates `ibex_top_tracing`, RAM, bus, UART-style output, simulator
control, and timer logic. `src/top_cpu/tb_ibex_cosim_bind.sv` binds the upstream
Ibex checker interface to this top, and `src/top_cpu/tb_ibex_cosim.cc` provides the
Verilator harness for the generated `Vtb_ibex` type. The checker still uses
`riscv_cosim_*` DPI calls from `external/ibex/dv/cosim`, but those calls are now
adapted to project `CosimSession` through a local `Cosim` compatibility wrapper
in `tb_ibex_cosim.cc`.

`CosimSession` now includes an Ibex-compatible interface for the wrapper:

```text
step_detail(write_reg, write_reg_data, pc, sync_trap, suppress_reg_write)
set_mip(pre_mip, post_mip)
set_nmi()/set_nmi_int()/set_debug_req()
set_mcycle()/set_csr()/get_csr()
notify_dside_access()/set_iside_error()
get_errors()/clear_errors()/get_insn_cnt()
```

## Firmware Cases

The default test is `hello`, so the default firmware artifacts are:

```text
build/firmware/hello/obj/firmware.elf
build/firmware/hello/obj/firmware.hex
```

PicoRV32 and VeeR EL2 use that shared firmware layout as their default fallback
when the runtime flow does not override the selected ELF or HEX path. The
`build.sh run` flow passes the same shared firmware artifacts into Ibex.

Active firmware cases are listed by `FIRMWARE_CASES`, which defaults to:

```text
hello pico_test mem
```

Shared firmware runtime code lives under `firmware/common`:

```text
firmware.mk
start.S
sections.lds
print.c
print.h
sim.c
sim.h
sim_show.h
```

Case directories contain case-specific program code and a small Makefile that
includes `firmware/common/firmware.mk`:

```text
firmware/hello/Makefile
firmware/hello/main.c
firmware/pico_test/Makefile
firmware/pico_test/main.c
firmware/mem/Makefile
firmware/mem/main.c
```

Running `make -C firmware/<case> firmware` creates local artifacts under that
case directory's `build/`. Running `make -C firmware/<case> install` publishes
the same ELF/HEX outputs to `build/firmware/<case>/obj`. `build.sh build
firmware` uses that install flow so existing CPU and SoC run commands keep the
same artifact paths. Each active case is self-checking and reports completion
through `sim_pass()` or `sim_fail()` at the shared firmware MMIO address.
`arith_basic_test` is no longer an active firmware case.

## Runtime Inputs

The Verilog testbench also has fallback values for firmware paths when plusargs
are not provided. `build.sh` passes explicit values during `./build.sh run` and
`./build.sh all`.

The shared firmware reset vector is `RESET_VECTOR`, defaulting to
`0x80000080`. PicoRV32 is configured with that value as `PROGADDR_RESET`.
Ibex derives `BootAddr` from the same value because the core computes its
post-reset PC as `{boot_addr_i[31:8], 8'h80}`.

### Firmware/TB Status Protocol

Firmware communicates with CPU and SoC testbenches by writing to the shared
simulation MMIO address:

```text
SIM_MMIO_ADDR = 0x10000000
```

The protocol values are defined in `firmware/common/sim.h`:

```text
0x80 RISCV_START   firmware started
0x81 RISCV_FINISH  normal finish marker
0x82 RISCV_FAIL    self-check failed
0x83 RISCV_PASS    self-check passed
0x00 RISCV_QUIT    stop request after PASS/FAIL/FINISH
```

Printable byte values written to the same address are treated as console output.
`sim_pass()`, `sim_fail()`, and `sim_finish()` write the status marker first,
execute an I/O fence, then write `RISCV_QUIT`. The fence keeps strongly ordered
status delivery visible to CPU and SoC monitors that observe external bus writes.

In SoC mode, `picosoc.v` routes `0x10000000` to `iomem_*` instead of SPI flash
or RAM. `tb_picosoc_soc.sv` records PASS/FAIL/FINISH when the write handshake is
seen, but it delays `spike_bus_finish()`/`$finish` until after the bus response
has been returned to Spike. This prevents a valid PASS/FAIL store from being
reported as a failed in-flight bus request. Timeout and Spike bus errors call
the same finish path with a non-zero exit code and are reported as FAIL.

`env.sh` has been removed. Override build settings with shell environment
variables before invoking `build.sh`; the script resolves and exports defaults
internally for subcommands.

## Troubleshooting

- `PICORV32_COSIM_IF=dpi` fails immediately:
  This is expected on the current Icarus flow. Use `PICORV32_COSIM_IF=vpi`.
- Cosim lifecycle mismatch (`init/retire/finish`) symptoms:
  Check wrapper mode, then confirm `log/*_cosim_result.log` and
  `log/*_spike_commit.log` are being generated for the active target.

## External Dependencies

The expected external dependency area is:

```text
external/riscv-pk/
external/picorv32/
external/ibex/
external/Cores-VeeR-EL2/
```

The current regression uses PicoRV32 RTL from
`external/picorv32/picorv32.v`. The `external/picorv32` directory is tracked as
a git submodule using the official upstream repository. The default Spike source
is the Ibex-compatible fork vendored under
`external/ibex/external/riscv-isa-sim`, and it is installed into `build/spike`.
pk is expected under `external/riscv-pk` and is installed into `build/pk`. A
fresh checkout must initialize submodules before running `./build.sh all`.

The Ibex target expects `external/ibex` to be available and uses FuseSoC to
build project core `local:spike_cosim:tb_ibex_cosim`, which depends on upstream
Ibex RTL and DPI cosim components. It uses the local `build/spike` installation
for `riscv-riscv`, `riscv-disasm`, and `riscv-fdt` pkg-config files by default.
Override `IBEX_PKG_CONFIG_PATH` only when intentionally testing another Spike
installation.

The VeeR EL2 target expects `external/Cores-VeeR-EL2` to be available. The
project does not modify vendor RTL for the cosim path; it binds the project
monitor into the upstream `tb_top` and keeps generated configuration, flists,
DTB, and Verilator output under `build/src/top_cpu/veer_el2`.

## Clean Behavior

`./build.sh clean` is a development clean. It removes generated firmware, top,
cosim, cache, and log outputs but preserves:

```text
build/spike
build/spike-build
build/pk
build/pk-build
```

`./build.sh clean-all` removes the whole `build/` directory plus generated
`log/` outputs.
