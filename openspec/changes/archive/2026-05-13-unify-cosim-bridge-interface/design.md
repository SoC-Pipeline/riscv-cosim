## Context

### Current bridge topology

```text
picorv32
  TB (Verilog) -> $cosim_* (VPI task) -> spike_dpi.cc -> CosimSession

ibex
  TB/bind (SV + C++) -> CosimAdapter methods -> CosimSession

veer_el2
  TB (SV) -> DPI-C veer_cosim_* -> tb_veer_el2_cosim.cc -> CosimSession
```

All three already converge at `CosimSession`, but the frontend bridge contracts are fragmented.

### Design target

Unify interface shape at bridge layer while preserving tool flexibility:

```text
                 +------------------------------+
                 | Unified C ABI Bridge         |
                 | cosim_bridge_*               |
                 +---------------+--------------+
                                 |
                           +-----v------+
                           | CosimSession|
                           +------------+
                           /     |      \
                  VPI shim      DPI shim   C++ adapter shim
```

## Goals / Non-Goals

Goals:
- Single canonical bridge API surface
- VPI retained for picorv32 baseline
- DPI prototype added for picorv32
- ibex / veer wrappers aligned to same bridge semantics

Non-goals:
- Enforcing one simulator (Verilator/Icarus/etc.)
- Removing proven working path before replacement is validated

## Unified Bridge Contract

Canonical lifecycle:

1. `bridge_create(config)` or `bridge_init(config)`
2. zero or more:
   - `bridge_retire(pc, instr)` for simple RVFI stream
   - `bridge_step_detail(...)` for ibex-style detailed checks
   - optional state sync calls (`set_csr`, `set_mip`, etc.)
3. `bridge_finish()`
4. `bridge_destroy()`

Contract rules:
- `finish()` must be idempotent
- `destroy()` must be safe after implicit simulator shutdown
- error state query must be accessible for all bridge frontends

### Entrypoint mapping inventory (implemented)

- VPI wrapper (`src/cosim/spike_dpi.cc`)
  - `$cosim_init` -> `cosim_bridge_init`
  - `$cosim_retire` -> `cosim_bridge_retire`
  - `$cosim_finish` -> `cosim_bridge_finish`
  - `$cosim_get_status` -> `cosim_bridge_pass_count`/`cosim_bridge_fail_count`
  - end-of-sim callback -> `cosim_bridge_reset`
- Ibex C++ adapter (`src/top/tb_ibex_cosim.cc`)
  - `Cosim::step` -> `cosim_bridge_step_detail`
  - `Cosim::set_mip/set_nmi/set_debug_req/set_mcycle/set_csr/...`
    -> corresponding `cosim_bridge_*`
  - `Cosim::get_errors/clear_errors/get_insn_cnt`
    -> `cosim_bridge_error_*`/`cosim_bridge_clear_errors`/`cosim_bridge_insn_count`
  - finish path -> `cosim_bridge_reset`
- VeeR EL2 DPI wrapper (`src/top/tb_veer_el2_cosim.cc`)
  - `veer_cosim_init` -> `cosim_bridge_init`
  - `veer_cosim_step` -> `cosim_bridge_retire`
  - `veer_cosim_finish` -> `cosim_bridge_finish`
  - `veer_cosim_get_fail_count` -> `cosim_bridge_fail_count`
  - `veer_cosim_reset` -> `cosim_bridge_reset`

### Compatibility alias strategy

- Keep wrapper-facing names stable during migration:
  - Verilog/system-task names remain `$cosim_*` for picorv32.
  - Existing Ibex `Cosim` interface remains unchanged to upstream checker code.
  - Existing VeeR DPI symbols remain unchanged.
- Internally, wrappers become thin aliases/forwarders to `cosim_bridge_*`.

### Error handling and idempotency rules

- `cosim_bridge_init` returns non-zero on invalid config or initialization exception.
- `cosim_bridge_retire`/`step_detail` return non-zero if session is unavailable or operation fails.
- `cosim_bridge_finish` is safe to call repeatedly; no active session is treated as success.
- `cosim_bridge_reset` is always safe and can be called after `finish` or on simulator shutdown hooks.
- Wrapper policy:
  - convert bridge return codes to wrapper-local diagnostics
  - avoid duplicate verification logic; treat `CosimSession` as the single source of truth

## Layering Plan

### Layer A: API normalization (no behavior change)
- Introduce/define canonical C ABI function set.
- Existing wrappers call this ABI.
- Keep current names as compatibility aliases during migration.

### Layer B: Frontend shim refactor
- VPI shim maps `$cosim_*` to canonical C ABI.
- DPI shim maps `*_cosim_*` imports to canonical C ABI.
- C++ adapters (ibex path) stop owning ad-hoc session lifecycle; delegate to canonical ABI.

### Layer C: picorv32 dual-path
- Keep `picorv32-vpi` as stable default.
- Add `picorv32-dpi` prototype top/path.
- Add build/run selector for interface mode.

### Layer D: decision gate
- Compare pass/fail stability, lifecycle correctness, and maintainability.
- If DPI path is robust: keep both or switch default (policy decision).
- If DPI path shows unresolved issues: keep both and document recommendations.

## Validation Matrix

Axes:
- Target: `picorv32`, `ibex`, `veer_el2`
- Bridge mode: VPI / DPI / C++ shim
- Test: `hello`, `pico_test` (where applicable)

Minimum required:
- No regressions in existing default flows
- picorv32 DPI prototype can complete at least baseline tests
- consistent bridge lifecycle behavior (init/retire/finish/destroy)

## Risks and Mitigations

1. **Tool capability mismatch (Icarus + DPI expectations)**
   - Mitigation: keep VPI baseline; treat DPI as optional path until proven.

2. **Semantic drift between wrappers**
   - Mitigation: wrappers become thin mapping only; business logic stays in `CosimSession`.

3. **Migration churn**
   - Mitigation: phased rollout, compatibility aliases, explicit mode selection.

## Decisions from validation

1. Current picorv32 runtime remains VPI on Icarus (`iverilog`/`vvp`).
2. `PICORV32_COSIM_IF=dpi` is kept as an explicit selector but currently fails fast with a clear diagnostic in `build.sh`.
3. `retire` and `step_detail` both remain first-class bridge operations to support simple and detailed frontends.
