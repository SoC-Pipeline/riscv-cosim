## Context

PicoRV32 CPU mode currently instantiates `picorv32_axi` with `RISCV_FORMAL` enabled and exposes RVFI retire data in `src/top_cpu/tb_picorv32.v`. The existing co-simulation path only calls `$cosim_retire(pc, instr)`, so Spike compares the retired PC and instruction but does not validate the DUT GPR writeback.

`tmp/cosim-arch-checker/mon/mon_instr` provides a useful monitor pattern: collect DUT architectural events and submit one retire transaction to a checker. Its backend is Whisper/CAC and DPI-oriented, while this repository uses Spike through an Icarus VPI path for PicoRV32. The monitor concept should be ported, but the backend must remain native to this repository.

## Goals / Non-Goals

**Goals:**
- Add a reusable `src/mon` instruction monitor layer for CPU-mode retire data.
- Integrate PicoRV32 through its RVFI retire interface, without modifying `external/picorv32/picorv32.v`.
- Compare PicoRV32 retired PC and GPR writeback against Spike using the existing `CosimSession::step_detail()` path.
- Structure the monitor transaction around resource changes so GPR and CSR data are represented closer to the original `monitor_gpr`/`monitor_csr` model.
- Capture PicoRV32 RVFI CSR read data for supported counter CSRs and make the compare policy explicit.
- Emit PicoRV32 monitor events to a dedicated log file.
- Keep the existing PicoRV32 Icarus/VPI flow and current firmware/TB pass/fail behavior working.
- Document the CPU-mode monitor architecture and first-stage coverage.

**Non-Goals:**
- Do not port Whisper, CAC, Bazel, or the full external checker backend.
- Do not convert PicoRV32 CPU mode to Verilator/DPI.
- Do not modify third-party PicoRV32 RTL for monitor instrumentation.
- Do not claim full CSR or memory consistency checking in this stage; memory fields may be logged for visibility and CSR compare is limited to stable masked reads.
- Do not claim FPR or vector-register checking for PicoRV32, which does not implement F/V resources.
- Do not expand the implementation to Ibex or VeeR EL2 in this change.

## Decisions

1. Use RVFI as the PicoRV32 data boundary.

   PicoRV32 already produces architectural retire data under `RISCV_FORMAL`, and the local wrapper already wires out PC, instruction, trap, GPR writeback, and memory fields. Observing RVFI keeps the monitor at the architectural boundary and avoids coupling to internal core implementation details.

2. Implement the PicoRV32 adapter at the testbench/wrapper level.

   The adapter belongs next to `picorv32_wrapper`, where RVFI signals and existing co-simulation lifecycle logic already meet. The third-party RTL remains unchanged, and the adapter can later be reused or replaced per DUT.

3. Port the monitor model, not the external backend.

   `src/mon` will carry the transaction/event abstraction and monitor logging. It will submit completed retire transactions to this repository's `cosim_bridge` instead of `tmp/cosim-arch-checker`'s Whisper/CAC bridge.

4. Use VPI tasks for PicoRV32.

   The current PicoRV32 build path explicitly supports VPI and rejects DPI. New monitor/detail tasks must be registered in `spike_dpi.cc` under `VPI_WRAPPER` and built into `libspike.vpi`.

5. Start with PC + GPR writeback comparison.

   `CosimSession::step_detail()` already compares the pre-step Spike PC and the single GPR writeback caused by the step. That is the deepest reliable check available without extending the Spike backend's memory/CSR comparison semantics.

6. Add CSR support as an optional resource on the retire packet.

   PicoRV32 RVFI exposes `mcycle`/`minstret` CSR read metadata as masked 64-bit values. The monitor should record these fields and pass them to the bridge as CSR read observations. Stable CSR reads may be compared against Spike using the existing CSR access capability. Volatile counter CSRs (`cycle`, `cycleh`, `instret`, `instreth`) are logged but not made mandatory compare points because PicoRV32 hardware counters and Spike instruction-step counters do not share a deterministic cycle basis.

7. Keep FPR/VR as future monitor resource types.

   The original `cosim-arch-checker` monitor has `monitor_fpr` and `monitor_vr`, but PicoRV32 is an integer RV32IMC target and the current `RiscvSimulator` abstraction exposes only XPR and CSR reads. FPR/VR support should wait for an F/V-capable DUT and explicit Spike state accessors.

## Risks / Trade-offs

- RVFI timing mismatch -> Keep the existing duplicate-retire guard and key it on retire identity/order so each instruction is stepped once.
- Interrupt/trap synchronization mismatch -> Pass RVFI trap information through the detail path and keep regression focused on PicoRV32 firmware cases before broadening scope.
- External monitor copy pulls in unwanted dependencies -> Only migrate the small transaction/monitor concept and adapt includes/build rules to this repository.
- CSR/memory coverage ambiguity -> Log available memory fields and compare only supported stable CSR observations; document volatile CSR exclusions.
- VPI argument handling is brittle -> Add a narrow system task with fixed-width scalar/vector arguments and keep failure reporting visible through existing cosim status.
- Resource event ordering -> Keep the PicoRV32 adapter aggregated around RVFI retire for now, while structuring the transaction so later targets can use separate `monitor_gpr`/`monitor_csr` event calls before retire.
