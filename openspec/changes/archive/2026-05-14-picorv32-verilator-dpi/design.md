## Context

PicoRV32 CPU-mode simulation is the remaining top-level flow that still uses `iverilog`/`vvp` plus VPI task callbacks for co-simulation. The existing `tb_picorv32` testbench already contains the required firmware loading, RVFI retire filtering, PASS/FAIL handling, timeout handling, and print MMIO behavior. The migration constraint is to keep `tb_picorv32` as the simulation top while removing the VPI task dependency and aligning PicoRV32 with the Verilator + DPI model already used elsewhere in this repository.

## Goals / Non-Goals

**Goals:**
- Keep `tb_picorv32` as the Verilator top-level testbench.
- Replace PicoRV32-specific `$cosim_*` VPI task calls with DPI-C imports.
- Preserve the current retire-monitor semantics by forwarding the full RVFI-derived retire payload into `mon_instr`.
- Replace the PicoRV32 `iverilog`/`vvp` build and run path with a Verilator executable flow in `build.sh`.

**Non-Goals:**
- Refactor the PicoRV32 testbench into an Ibex-style external clock/reset top.
- Change firmware format, memory map, or shared PASS/FAIL MMIO protocol.
- Redesign shared cosim backend behavior for other CPUs or SoC-mode flows.

## Decisions

### Keep `tb_picorv32` as the top and use Verilator timing mode

`tb_picorv32` already contains behavioral timing constructs (`always #5`, `#1`, `#2`) that define reset release, retire sampling, and finish ordering. Instead of restructuring the testbench around a C++-driven clock/reset interface, the Verilator build will compile `tb_picorv32` directly with `--timing`.

Alternative considered:
- Rework PicoRV32 into a wrapper top with clock/reset IO driven from C++. Rejected because it broadens the migration and risks changing long-standing testbench behavior that is unrelated to the VPI removal.

### Replace PicoRV32 VPI entrypoints with a PicoRV32-specific DPI implementation

The current VPI tasks (`$cosim_init`, `$cosim_monitor_retire`, `$cosim_finish`) will be replaced in `tb_picorv32` with PicoRV32-specific DPI-C imports implemented in a new `tb_picorv32_cosim.cc` file. This keeps the top-level testbench logic intact while removing the VPI registration layer entirely.

Alternative considered:
- Keep a thin VPI shim and forward internally to DPI/C++ helpers. Rejected because it preserves the VPI dependency and does not meet the maintenance goal.

### Preserve full RVFI retire payload into `MonInstrTxn`

PicoRV32 currently computes a detailed retire payload including GPR, memory, and CSR activity before calling `$cosim_monitor_retire`. The new DPI callback will keep the same argument shape and reconstruct `MonInstrTxn` in C++, then call `mon_instr_retire`. This preserves current monitoring fidelity and avoids a semantic downgrade to a `pc/instr`-only retire API.

Alternative considered:
- Collapse PicoRV32 to a simplified retire-only API like the early `DPI_WRAPPER` path in `spike_dpi.cc`. Rejected because it would drop the detailed monitor coverage that the current PicoRV32 flow already has.

### Move PicoRV32 top-level DPI glue out of `spike_dpi.cc`

`spike_dpi.cc` currently mixes shared backend code with historical PicoRV32 VPI/DPI wrappers. The PicoRV32 top-specific DPI entrypoints and Verilator `main()` will live in `src/top_cpu/tb_picorv32_cosim.cc`, matching the repository pattern used by `tb_veer_el2_cosim.cc`. Shared cosim backend code remains in `src/cosim/`.

Alternative considered:
- Extend `spike_dpi.cc` again with another PicoRV32-specific DPI mode. Rejected because it keeps top-level simulation glue coupled to a legacy wrapper file that should be shrinking, not growing.

## Risks / Trade-offs

- **[Timing-sensitive finish ordering]** PicoRV32 relies on delayed retire/finish scheduling in the testbench. → Mitigation: keep the existing SV-side retire and finish sequencing and compile with Verilator `--timing`.
- **[Behavior drift during simulator switch]** Changing from `vvp` to Verilator may expose different scheduling edges or exit behavior. → Mitigation: preserve existing top-level testbench semantics and run focused PicoRV32 regressions for PASS/FAIL, timeout, and firmware load.
- **[Build logic overlap during transition]** `build.sh` currently assumes PicoRV32 needs `libspike.vpi`. → Mitigation: split PicoRV32 build/run logic from legacy VPI artifacts and scope VPI generation only to remaining users, if any.

## Migration Plan

1. Add the PicoRV32-specific DPI C++ harness and replace VPI calls in `tb_picorv32`.
2. Switch PicoRV32 build/run commands in `build.sh` from `iverilog`/`vvp` to Verilator executable generation.
3. Validate focused PicoRV32 CPU-mode cases and confirm VPI is no longer required for that path.
4. Leave unrelated CPU/SoC flows unchanged.

## Open Questions

No blocking open questions remain for implementation. The remaining work is mechanical integration and regression verification.
