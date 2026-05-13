## Why

The active co-simulation build now has a small linear flow, but it is still hidden inside a Makefile that also carries inactive legacy PicoRV32 targets. Replacing it with `build.sh` makes the supported commands explicit and avoids keeping stale Makefile entry points.

## What Changes

- Add `build.sh` as the supported build entry point.
- Make `./build.sh` with no arguments print help and exit successfully.
- Support active commands for `help`, `firmware`, `vpi`, `sim`, `all`, and `clean`.
- Preserve the current active co-simulation behavior, generated output layout, and environment override model.
- Update project documentation and TODO guidance from `make clean && make all` to `./build.sh clean && ./build.sh all`.
- Remove `Makefile` after `build.sh` passes the current regression.

## Capabilities

### New Capabilities

- `shell-build-entrypoint`: Defines `build.sh` as the supported build, simulation, and cleanup entry point.

### Modified Capabilities

None.

## Impact

- Affects `build.sh`, `Makefile`, `README.md`, `docs/arch.md`, and `TODO.md`.
- Removes inactive legacy Makefile targets instead of porting them.
- Keeps source, firmware, VPI, and simulation behavior unchanged for the active default case.
