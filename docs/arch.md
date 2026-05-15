# Architecture

This document describes how the repository is wired internally. For command usage, selectors, environment overrides, and clean behavior, use `./build.sh help` and the top-level `README.md`.

## Modes

The project has two execution modes with different validation purposes.

```text
                  CPU retire-compare mode
                  =======================

 firmware ELF/HEX
        |
        v
 target CPU testbench + DUT
        |
        | retire PC/instruction/events
        v
 cosim_bridge (VPI/DPI C ABI)
        |
        v
 CosimSession
        |
        v
 SpikeSimulator
        |
        v
 compare counters + logs
```

```text
                  SoC bus-driven mode
                  ===================

 firmware ELF -> firmware.hex
        |
        v
 TB $readmemh into PicoSoC RAM
        |
        v
 reset release
        |
        v
 Spike bus master replaces PicoRV32 CPU slot
        |
        | fetch/load/store bus transactions
        v
 PicoSoC RAM / UART-SPI regs / sim MMIO
        |
        v
 firmware PASS/FAIL/QUIT writes end simulation
```

CPU mode checks a DUT CPU against Spike at retire time. SoC mode removes the DUT CPU from the execution path and validates that a Spike-driven bus master can execute firmware through the PicoSoC memory map and devices.

## CPU Mode Data Flow

CPU-mode testbenches live under `src/top_cpu/`. They each provide a target-specific wrapper around the same reference comparison layer.

### PicoRV32

```text
firmware.hex
  -> tb_picorv32.v memory preload
  -> PicoRV32 DUT executes firmware
  -> RVFI retire observation in tb_picorv32.v
  -> src/mon/mon_instr transaction logging
  -> VPI calls in src/cosim/spike_dpi.cc
  -> CosimSession::step_detail()
  -> SpikeSimulator golden state
```

PicoRV32 uses Icarus Verilog plus the project VPI module. The build artifact name is still `libspike`, but the Verilog-facing tasks are a project cosim interface, not a raw Spike-only API.

PicoRV32 is the first CPU target wired through the monitor path. The DUT data boundary is the RVFI retire interface emitted by `picorv32_axi` with `RISCV_FORMAL` enabled; the project does not modify `external/picorv32/picorv32.v` to add cosim instrumentation. On each known `rvfi_valid` retire, the wrapper reports the retire order, PC, instruction, trap flag, GPR writeback, RVFI memory fields, and supported RVFI CSR observations to `$cosim_monitor_retire`. The monitor writes the full packet to `PICORV32_MON_LOG` and submits PC plus GPR writeback, with optional CSR metadata, to `CosimSession::step_detail()`.

The first-stage PicoRV32 monitor check covers retired PC and the single architectural GPR writeback for the instruction. RVFI memory fields are logged for debug and future extension, but memory consistency is not yet enforced by `CosimSession`. PicoRV32 also exposes RVFI counter CSR reads for `cycle`, `cycleh`, `instret`, and `instreth`; those observations are logged and carried through the monitor transaction, but they are treated as volatile and do not cause compare failures on their own. FPR and vector-register monitor resources remain future work for F/V-capable targets.

### Ibex

```text
firmware.elf
  -> Ibex simple-system RAM load
  -> ibex_top_tracing retire/checker signals
  -> upstream-style riscv_cosim_* DPI calls
  -> local compatibility wrapper
  -> CosimSession
```

The Ibex target keeps compatibility with the upstream Ibex DPI checker surface while adapting it to the project `CosimSession` implementation.

### VeeR EL2

```text
firmware.elf + firmware_veer.hex
  -> upstream VeeR EL2 tb_top memory load
  -> project monitor observes trace_rv_i_* retire signals
  -> Verilator DPI adapter
  -> CosimSession
```

The project monitor binds into the upstream VeeR testbench without modifying vendor RTL for the cosim path.

## Cosim Layering

```text
┌─────────────────────────────────────────────────────────────┐
│                    Testbench / Monitor                      │
│      PicoRV32 VPI       Ibex DPI        VeeR EL2 DPI        │
└──────────────┬──────────────┬──────────────┬────────────────┘
               │              │              │
               v              v              v
      ┌────────────────────────────────────────────┐
      │              cosim_bridge                  │
      │       C ABI wrappers for VPI/DPI users     │
      └────────────────────┬───────────────────────┘
                           v
      ┌────────────────────────────────────────────┐
      │              CosimSession                  │
      │ retire comparison, CSR/debug/IRQ sync, logs│
      └────────────────────┬───────────────────────┘
                           v
      ┌────────────────────────────────────────────┐
      │             SpikeSimulator                 │
      │       processor_t backend and simif_t      │
      └────────────────────────────────────────────┘
```

Layer responsibilities:

- `cosim_bridge`: C ABI boundary for DPI/VPI users; it should remain thin.
- `CosimSession`: verification policy, retire comparison, counters, and log generation.
- `RiscvSimulator`: backend-neutral reference simulator interface.
- `SpikeSimulator`: current backend implementation using Spike `processor_t`.
- `SimulatorCapabilities`: feature flags used when a backend cannot support a synchronization surface.

The current default backend is Spike. Adding another backend should mainly require a new `RiscvSimulator` subclass and a factory registration in `src/cosim/simulator_factory.cc`.

## SoC Mode Structure

SoC mode is based on a project-owned copy of PicoSoC in `src/top_soc/picosoc.v`. The upstream SoC structure is kept intentionally recognizable: RAM, SPI flash interface, UART registers, and external IO path remain, while the original PicoRV32 CPU instance is replaced by `spike_bus_master`.

```text
                       src/top_soc/tb_picosoc_soc.sv
                                      |
                                      | $readmemh firmware.hex
                                      v
┌────────────────────────────────────────────────────────────────┐
│                         picosoc.v                              │
│                                                                │
│  ┌──────────────────┐       ┌────────────────────────────────┐ │
│  │ spike_bus_master │──────▶│ PicoSoC bus decode             │ │
│  └────────┬─────────┘       │ - RAM at 0x80000000            │ │
│           │                 │ - SPI/UART regs at 0x02000000  │ │
│           │                 │ - sim MMIO at 0x10000000       │ │
│           │                 └───────────────┬────────────────┘ │
│           │                                 │                  │
│           v                                 v                  │
│  ┌──────────────────┐             ┌──────────────────────────┐ │
│  │ spike_bus_dpi.cc │             │ picosoc_mem / devices    │ │
│  └──────────────────┘             └──────────────────────────┘ │
└────────────────────────────────────────────────────────────────┘
```

Bus roles:

- Master: Spike through `spike_bus_master.sv` and `spike_bus_dpi.cc`.
- Slaves: PicoSoC RAM, PicoSoC SPI/UART registers, and TB-handled simulation MMIO.

SoC address map used by this repository:

```text
0x80000000... RAM
0x02000000... PicoSoC SPI/UART register region
0x10000000    shared firmware/TB simulation MMIO
```

The simulation MMIO address is decoded explicitly before the SPI flash path. This keeps firmware status and console writes visible to `tb_picosoc_soc.sv` through `iomem_valid/iomem_wstrb/iomem_addr/iomem_wdata`.

## Firmware And HEX Loading

Firmware cases are self-checking programs under `firmware/`. Shared runtime code lives in `firmware/common`.

```text
firmware/common/start.S       reset entry, stack setup, sim_start, main handoff
firmware/common/sections.lds  RAM origin and reset vector placement
firmware/common/print.c       console output through sim MMIO
firmware/common/sim.c         START/PASS/FAIL/FINISH/QUIT protocol helpers
firmware/common/firmware.mk   reusable firmware build rules
firmware/<case>/main.c        case-specific self-check
```

The default shared cases are `hello`, `pico_test`, and `mem`. Each case reports success through `sim_pass()` and failure through `sim_fail()`. PicoRV32-specific monitor validation can add target-focused cases such as `pico_csr` without changing the default cross-target `all` matrix.

### CPU Mode Loading

CPU mode consumes the same firmware artifacts, but each target loads them through its own testbench convention:

```text
PicoRV32: firmware.hex      -> tb_picorv32 memory array
Ibex:     firmware.elf      -> Verilator --meminit RAM flow
VeeR EL2: firmware_veer.hex -> upstream VeeR memory image flow
```

All CPU targets use the same firmware reset convention: RAM at `0x80000000` and reset entry at `0x80000080`.
`build.sh` also tracks firmware source, common runtime sources, linker script, and per-case Makefiles with a stamp file under `build/firmware/<case>/obj/.build.stamp`, so a changed firmware source is rebuilt automatically before the next run.

### SoC Mode Loading

SoC mode deliberately has a single firmware image loader: the SystemVerilog testbench.

```text
firmware.elf
  -> firmware.hex
  -> tb_picosoc_soc.sv $readmemh
  -> uut.memory.mem
  -> resetn release
  -> Spike PC set to ELF entry
  -> Spike fetch/load/store requests go through the SoC bus
```

`spike_bus_dpi.cc` does not preload ELF or HEX contents into private simulator memory. Its `simif_t::addr_to_mem()` returns `nullptr`, forcing instruction fetches and data accesses through the DPI bus bridge. The SoC TB prints RAM read/write counters so a run can show that execution used the PicoSoC RAM path rather than hidden Spike memory.

## Firmware/TB Protocol

Firmware communicates with all CPU and SoC testbenches through one shared MMIO address:

```text
SIM_MMIO_ADDR = 0x10000000
```

Protocol byte values are defined in `firmware/common/sim.h`:

```text
0x80 RISCV_START   firmware started
0x81 RISCV_FINISH  normal finish marker
0x82 RISCV_FAIL    self-check failed
0x83 RISCV_PASS    self-check passed
0x00 RISCV_QUIT    stop request after PASS/FAIL/FINISH
```

Printable byte values written to the same address are treated as console output.

`sim_pass()`, `sim_fail()`, and `sim_finish()` write the status marker first, execute an I/O fence, then write `RISCV_QUIT`. The fence is important for CPU monitors that observe external bus writes; it prevents the terminal `QUIT` write from becoming visible without the preceding status marker.

In SoC mode, the TB records PASS/FAIL/FINISH when the MMIO write handshake is seen, then waits for the following `RISCV_QUIT` write before calling `spike_bus_finish()` and `$finish`. This avoids stopping the Spike worker while the status store is still waiting for its bus response. Timeout and Spike bus errors use the same finish path with a non-zero exit code.

## Logs And Evidence

The run flow writes three kinds of evidence:

```text
log/run_<target>_<case>.log              target stdout/stderr capture
log/<target>_cosim_result.log            retire compare result log
log/<target>_spike_commit.log            Spike commit log when enabled
log/picorv32_mon.log                     PicoRV32 RVFI monitor packet log
log/run_soc_spike_<case>.log             SoC-mode stdout/stderr capture
log/soc_spike_<case>_commit.log          SoC-mode Spike commit log
```

For SoC mode, runtime RAM access counters are printed as:

```text
[SOC] ram_reads=<N> ram_writes=<N>
```

Those counters are collected from `uut.mem_valid && uut.mem_ready && uut.ram_sel` inside the SoC TB, so they are a direct check that Spike fetched and accessed through PicoSoC RAM.

## Notes For Extending The System

When adding a CPU target, keep the target-specific tracing and memory-load convention in `src/top_cpu`, then adapt retire events into `CosimSession` through the existing bridge surface.

When adding a simulator backend, keep wrappers stable and add a backend behind `RiscvSimulator`. Be explicit about `SimulatorCapabilities`; `CosimSession` uses those flags to decide whether instruction fetch comparison, CSR access, interrupt sync, and debug request sync are available.

When adding a firmware case, use the shared runtime helpers and make the case self-checking. A passing case should call `sim_pass()`, and a failing case should call `sim_fail()`.
