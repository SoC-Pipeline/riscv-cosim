## Why

The active regression currently writes generated firmware objects, simulation binaries, and VPI modules back into source directories. Moving those artifacts under `build/` keeps `firmware/`, `src/`, and `scripts/` as source/input areas and makes cleanup predictable.

## What Changes

- Route active firmware build outputs from `firmware/<TEST_NAME>/obj/` to `build/firmware/<TEST_NAME>/obj/`.
- Route generated source-side outputs from `src/top/` and `scripts/` to `build/src/top/` and `build/src/cosim/`.
- Keep source inputs under `firmware/`, `src/`, and `scripts/` unchanged.
- Update testbench fallback paths, environment exports, documentation, and clean behavior to match the new generated-output layout.
- Preserve `make all` as the primary regression entry point and preserve explicit plusarg behavior for ELF, HEX, PK, and ISA paths.

## Capabilities

### New Capabilities

- `build-artifact-layout`: Defines where active co-simulation generated artifacts are produced and consumed.

### Modified Capabilities

None.

## Impact

- Affects `Makefile`, `env.sh`, `README.md`, `docs/arch.md`, and `src/top/testbench.v`.
- Changes generated output paths for firmware ELF/HEX/BIN/LST/object files, Icarus `.vvp`, and Spike VPI `.so`/`.vpi`.
- Existing source files remain in place; no firmware, RTL, or Spike behavior is intended to change.
