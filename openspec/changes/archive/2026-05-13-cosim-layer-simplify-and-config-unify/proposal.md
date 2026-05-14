## Why

`src/cosim` currently contains an active bridge/session/simulator path and a legacy threaded path. The legacy path is not used by current build targets and increases maintenance burden.

Configuration and log path assembly are also repeated across wrappers (`spike_dpi.cc`, `tb_ibex_cosim.cc`, `tb_veer_el2_cosim.cc`), making behavior drift likely when adding new targets.

## What Changes

1. Remove unused legacy cosim files (`spike_dpi_thread.cc`, `sim.h`, `htif.h`).
2. Unify cosim config construction through a shared helper keyed by `cpu_name`.
3. Unify default log naming policy:
   - `dump/<cpu_name>_cosim_result.log`
   - `dump/<cpu_name>_spike_commit.log`
4. Keep compatibility with existing environment overrides while reducing per-wrapper duplication.
5. Update docs and build help to match the unified policy.

## Scope

In scope:
- `src/cosim`, `src/top/*cosim*.cc`, `build.sh`, docs updates
- Regression runs for `picorv32/ibex/veer_el2` with `hello` and `pico_test`

Out of scope:
- Changing co-sim comparison semantics in `CosimSession`
- Replacing existing wrappers with a single simulator-specific ABI

## Risks

- Wrapper behavior regressions from refactoring initialization.
- Env var compatibility break if precedence changes unexpectedly.

Mitigation:
- Small-step changes with test after each step.
- Preserve existing env names as compatibility layer.
