## Context

PicoRV32 already reports retire packets through the local `MonInstrTxn` schema
 and writes a dedicated monitor log before forwarding key retire fields into
`CosimSession::step_detail_with_csr()`. Ibex already has a richer upstream
checker path with RVFI retire data, interrupt/debug sideband sync, and dside
memory observation. VeeR EL2 currently exposes only public trace retire signals
to the local cosim path, so its compare surface is much thinner.

The change needs to improve monitor consistency without regressing the working
Ibex checker or forcing unstable VeeR internal taps into the first landing.

## Goals / Non-Goals

**Goals:**
- Reuse the existing `MonInstrTxn` and `mon_instr` runtime outside PicoRV32.
- Add Ibex monitor logging without removing the current checker semantics.
- Move VeeR EL2 retire events onto the monitor path with the public trace data
  available today.
- Keep follow-up VeeR GPR/CSR monitor work explicitly staged and tracked.

**Non-Goals:**
- Replacing the Ibex checker with a monitor-only flow in the first landing.
- Delivering full FPR, vector-register, or generic memory-resource compare.
- Delivering VeeR hierarchical CSR compare in the first landing.

## Decisions

### 1. Keep Ibex's existing checker as the source of truth

Ibex already feeds richer sideband state into the co-simulator than PicoRV32.
The first implementation will keep `riscv_cosim_step()`, `set_*()`, and
`notify_dside_access()` unchanged, and will add monitor logging in parallel.

Alternative considered:
- Replace the Ibex checker with the monitor path immediately.
  Rejected because it would risk losing current interrupt/debug/dside behavior.

### 2. Make `mon_instr` target-agnostic

The monitor runtime will accept an explicit log path during initialization so
Ibex and VeeR EL2 can produce target-specific monitor logs without depending on
the PicoRV32 VPI wrapper defaults.

Alternative considered:
- Add separate monitor implementations per target.
  Rejected because it duplicates the transaction schema and log format.

### 3. Land VeeR EL2 in two stages

The first VeeR landing will build `MonInstrTxn` from the existing public trace
signals: order, PC, instruction, and trap indication. Follow-up tasks can then
add writeback resources from stable testbench mirrors before considering any
deeper internal taps.

Alternative considered:
- Bind directly to internal `dec_*` writeback/CSR signals in the first landing.
  Rejected because the timing/alignment risk is higher and would enlarge the
  initial debug surface.

## Risks / Trade-offs

- [Ibex double-stepping risk] -> The new monitor path must not call
  `CosimSession::step_detail*()` in parallel with the existing checker. The
  first Ibex landing will log monitor transactions only.
- [VeeR visibility gap] -> The VeeR compare surface remains `pc/instr` only
  even after GPR/CSR writeback logging is added. This is accepted for now; the
  shared monitor log is richer than the active compare contract.
- [Target log sprawl] -> More monitor logs will be produced. Use explicit,
  target-named defaults to keep evidence readable.
- [Hierarchical VeeR fragility] -> Internal taps below the existing `tb_top`
  writeback mirrors may break on upstream RTL refresh. Keep those taps
  project-local and isolated to the monitor bind, and avoid landing them unless
  the timing contract is clear.
