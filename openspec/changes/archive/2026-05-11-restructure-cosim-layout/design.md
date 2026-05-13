## Context

The current repository keeps active co-simulation assets in `top/`, `tests/`, and `scripts/`. The active regression path is `make all`, which runs `build2`, builds the VPI bridge, then launches Icarus Verilog with the Spike bridge. That flow depends on hard-coded references to `tests/arith_basic_test`, `top/testbench.v`, `top/picorv32.v`, and `scripts/spike_dpi.cc`.

The target layout separates project-owned RTL and bridge code under `src/`, firmware inputs under `firmware/`, external dependency drops under `external/`, and Python utilities under `scripts/`.

## Goals / Non-Goals

**Goals:**
- Align the repository layout with `TODO.md`.
- Preserve `make all` as the primary co-simulation regression command.
- Keep Python utilities in `scripts/`.
- Update documentation so new contributors can identify the main source, firmware, external, and generated-output areas.
- Keep changes minimal and avoid unrelated formatting edits.

**Non-Goals:**
- Do not rewrite the co-simulation algorithm.
- Do not change firmware contents or PicoRV32 behavior.
- Do not vendor or modify external dependencies beyond placing them under `external/`.
- Do not redesign generated artifact locations beyond what is needed for path correctness.

## Decisions

- Use `src/top/` for Verilog RTL and testbench files.
  - Rationale: This preserves the existing `top` grouping while placing project-owned source under `src/`.
  - Alternative considered: flattening Verilog into `src/`; rejected because the current grouping is clear and referenced by TODO.md.

- Use `src/cosim/` for C/C++ bridge sources and headers moved from `scripts/`.
  - Rationale: The bridge is source code, not a script, and needs a clear namespace separate from RTL.
  - Alternative considered: placing bridge files directly under `src/`; rejected to avoid mixing Verilog and C++ files at the same level.

- Keep Python utilities under `scripts/`.
  - Rationale: TODO.md explicitly says `*.py` remain in `scripts/`, and the Makefile already calls them as tools.

- Keep `libspike.so` and `libspike.vpi` output under `scripts/` initially.
  - Rationale: This keeps `vvp -M scripts -m libspike` stable and limits the migration blast radius.
  - Alternative considered: moving generated VPI artifacts into `build/`; deferred because it would require broader clean and load-path changes.

- Parameterize common directories in `Makefile`.
  - Rationale: Variables for firmware, RTL, cosim source, script, and dump directories reduce future path drift.

## Risks / Trade-offs

- Hard-coded paths may remain in comments or inactive legacy Makefile targets -> mitigate by searching for `tests/`, `top/`, and `scripts/` references after migration.
- Moving C++ headers can change include resolution -> mitigate by adding the new `src/cosim` include directory to the VPI build.
- External `picorv32` may not exist yet -> mitigate by documenting it as the expected location without requiring it for the current regression, since the active RTL copy remains project-owned under `src/top/`.
- Existing generated files may remain in old directories -> mitigate with clean target updates and regression from a clean-enough state.

## Migration Plan

1. Move tracked RTL files into `src/top/`.
2. Move tracked firmware test directories into `firmware/`.
3. Move tracked C/C++ bridge files into `src/cosim/`, leaving Python utilities in `scripts/`.
4. Update `Makefile`, `env.sh`, and Verilog fallback paths.
5. Add `docs/arch.md` and refresh README path guidance.
6. Run `make all` with the required modules loaded.
7. Review `git diff` and ensure OpenSpec tasks reflect the completed work.
