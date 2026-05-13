## Why

The current `build.sh sim` and `all` flows mix dependency builds, firmware
builds, top builds, and execution. This makes routine clean/regression work
rebuild Spike and pk unnecessarily and obscures which artifact each command
owns.

## What Changes

- **BREAKING** Replace the old top-level `firmware`, `spike`, `pk`, `deps`,
  `vpi`, and `sim` commands with explicit `build` and `run` commands.
- Add `./build.sh build spike` to build local Spike, pk, and the PicoRV32 VPI
  module.
- Add `./build.sh build -f [case|all]` to build firmware cases, defaulting to
  the active firmware case list.
- Add `./build.sh build -t [picorv32|ibex|all]` to build top-level simulation
  artifacts.
- Add `./build.sh run [picorv32|ibex|all] [case|all]` to execute already
  defined target/case combinations, building required artifacts first.
- Keep `./build.sh all` as the regression entry point over active firmware
  cases and both supported targets.
- Change `./build.sh clean` to remove generated firmware, top, cosim, dump, and
  cache outputs while preserving Spike and pk build/install directories.
- Add `./build.sh clean-all` to remove the whole `build/` directory and `dump/`.
- Make Ibex use the local `build/spike` installation by default.

## Capabilities

### New Capabilities

- `build-cli`: Command-line build/run/clean behavior for project artifacts and
  simulations.

### Modified Capabilities

- None.

## Impact

- `build.sh`
- `README.md`
- `docs/arch.md`
- OpenSpec tasks and verification commands
