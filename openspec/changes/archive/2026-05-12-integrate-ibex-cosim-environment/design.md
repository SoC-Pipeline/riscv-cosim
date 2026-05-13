## Context

The active project regression is PicoRV32 through Icarus Verilog and a
project-owned VPI bridge. Ibex has a separate upstream Verilator cosim
environment under `external/ibex/dv/verilator/simple_system_cosim` that uses
Ibex extended RVFI, DPI calls, and a SpikeCosim checker. That checker validates
more than PC/instruction retirement: it also tracks register writes, memory
accesses, traps, interrupts, debug requests, and selected CSRs.

The first Ibex integration should therefore run the upstream environment as an
independent target. Unifying the Ibex DPI checker with the existing PicoRV32
`$cosim_*` VPI tasks before the Ibex target is runnable would add unnecessary
risk.

## Goals / Non-Goals

**Goals:**

- Provide `./build.sh sim ibex` as a project-level entry point for Ibex
  Verilator co-simulation.
- Preserve the existing PicoRV32 simulation behavior and default command shape.
- Build a firmware image from the existing firmware test sources for the Ibex
  simple-system memory map.
- Run Ibex with its upstream simple-system cosim checker and confirm a clean
  cosim result.
- Keep generated Ibex build outputs under project `build/`.
- Document the Ibex target, dependency expectations, and target selection.

**Non-Goals:**

- Do not merge Ibex DPI cosim into the PicoRV32 `$cosim_*` VPI API in this
  change.
- Do not rewrite the Ibex simple-system bus, RAM, timer, simulator control, or
  checker logic.
- Do not remove or rename PicoRV32 build targets.
- Do not require `./build.sh sim ibex` to rebuild Spike from source unless the
  required pkg-config metadata is missing.

## Decisions

1. Keep Ibex as a separate simulation target.

   `./build.sh sim` remains the PicoRV32 regression for compatibility. Passing
   `ibex` selects the Verilator/FuseSoC-backed Ibex flow. This keeps the initial
   integration easy to test and avoids changing the existing Icarus path.

2. Use the upstream Ibex simple-system cosim checker first.

   The checker already binds to `ibex_simple_system`, creates SpikeCosim through
   DPI, copies RAM contents into Spike, and checks Ibex RVFI plus D-side memory
   events. A project wrapper can reference this environment, but the first
   working target should avoid copying or hand-porting the checker internals.

3. Add an Ibex firmware variant rather than forcing one ELF to support both
   systems.

   The current PicoRV32 firmware links at `0x80000000` and terminates by
   writing `123456789` to `0x20000000`. Ibex simple-system uses RAM at
   `0x00100000`, starts execution at `0x00100080`, outputs characters through
   `0x00020000`, and halts through `0x00020008`. The build should produce an
   Ibex-specific ELF from the existing test sources with the Ibex address and
   halt conventions.

4. Prefer FuseSoC for the initial Ibex Verilator build.

   Ibex already encodes its required SystemVerilog, DPI, C++, include, waiver,
   and linker flags in core files. Calling FuseSoC keeps the first integration
   close to upstream and reduces the chance of missing generated dependencies.
   Later changes can decide whether to vendor a flattened file list under
   `src/`.

5. Resolve Spike pkg-config metadata explicitly.

   Ibex cosim requires `riscv-riscv`, `riscv-disasm`, and `riscv-fdt` packages.
   The build should set or validate `PKG_CONFIG_PATH` for the selected Ibex
   cosim Spike installation and fail with a clear message when it is missing.

## Risks / Trade-offs

- [Risk] Current project Spike build may not have the Ibex cosim branch or
  `riscv-fdt.pc`. → Mitigate by detecting pkg-config availability and allowing
  `IBEX_SPIKE_PREFIX` / `PKG_CONFIG_PATH` overrides.
- [Risk] The existing arithmetic firmware may contain PicoRV32-specific MMIO or
  assumptions. → Mitigate with an Ibex-specific linker/startup/termination
  variant while reusing the same test source where practical.
- [Risk] FuseSoC may place outputs under its default build tree. → Mitigate by
  passing a project build root or documenting the generated Ibex subdirectory.
- [Risk] Verilator version sensitivity. → Mitigate by documenting the requested
  module environment and verifying with the available toolchain.
- [Risk] `./build.sh clean` removes generated dependency outputs. → Mitigate by
  making `./build.sh sim ibex` build any target-local generated outputs it needs
  and fail clearly if external dependencies are absent.
