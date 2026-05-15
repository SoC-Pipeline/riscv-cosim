## Context

The current CPU-mode co-sim stack already has a good architectural split: target-specific RTL observation feeds shared monitor and cosim layers. The duplication sits one layer above that split, inside the target-specific DPI glue.

Today, PicoRV32, Ibex, and VeeR EL2 each repeat some combination of:

- resolve environment-backed log paths
- create parent directories for logs
- initialize `MonInstr`
- build simple monitor retire packets
- finish/reset shared monitor state

At the same time, the three targets differ materially in the parts closest to the DUT:

- PicoRV32 uses event-driven Verilator stepping and full RVFI monitor packets
- Ibex keeps the upstream checker flow as the authoritative compare path
- VeeR EL2 uses explicit clock stepping and retire-trace-based monitor input

That means the right refactor boundary is the helper layer around shared monitor/cosim primitives, not a new common runtime framework.

## Goals / Non-Goals

**Goals:**
- Remove low-value duplication from top-level CPU DPI glue.
- Make log-path handling and monitor setup consistent across targets.
- Provide a shared retire-only monitor submission path for targets that do not have full RVFI packets.
- Preserve current target-specific execution flow and compare behavior.

**Non-Goals:**
- Do not unify Verilator `main()` loops across targets.
- Do not refactor target-specific RTL observation or retire de-duplication logic.
- Do not change Ibex upstream checker integration.
- Do not introduce a generic CPU cosim base class or large runtime abstraction.

## Decisions

### 1. Extend `cosim_top_utils` with parent-path handling

Add a shared `top_ensure_parent_directory(path)` helper and use it from all top-level CPU glue code.

Why:
- The logic already exists three times with identical behavior.
- This keeps path and directory semantics in one place without introducing a new abstraction layer.

Alternative considered:
- Keep per-file `EnsureParentDirectory()` helpers.
  - Rejected because it preserves repeated bug-fix sites with no target-specific value.

### 2. Add a retire-only helper on the monitor side

Extend `mon_instr` with a helper that submits a `pc/instr/trap` retire packet without requiring each caller to hand-build a mostly empty `MonInstrTxn`.

Why:
- VeeR currently builds a sparse transaction by hand.
- The helper aligns with the existing `mon_instr_retire_simple(...)` pattern used by Ibex.

Alternative considered:
- Introduce a generic `MonInstrTxnBuilder` class.
  - Rejected because that is more abstraction than the current call sites need.

### 3. Keep full-packet construction target-specific

PicoRV32 will continue to construct its full RVFI-backed `MonInstrTxn` locally.

Why:
- The RVFI packet shape is target-specific and materially richer than the retire-only path.
- Folding that into a common builder would increase coupling without reducing much code.

Alternative considered:
- Unify all monitor submissions through a single builder API.
  - Rejected because the packet richness differs too much across targets.

### 4. Do not unify target runtimes

The refactor will not change PicoRV32 event stepping, VeeR clock stepping, or the Ibex `VerilatorSimCtrl` wrapper.

Why:
- These are target-specific runtime models, not accidental duplication.
- Forcing them into a single framework would hide real differences and raise debug cost.

## Risks / Trade-offs

- [Helper extraction drifts into framework design] → Keep the shared layer to free functions and small data helpers only.
- [Monitor helper semantics become too generic] → Limit the new helper to retire-only submission and leave full-packet construction local.
- [Ibex glue gets over-normalized] → Only touch its local monitor helper path; leave the upstream checker flow unchanged.
