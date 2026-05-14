# Proposal: Migrate PicoSoC CPU Slot to Spike Bus Master

## Why

Current SoC mode runs Spike from a C++ testbench top. This validates bus behavior,
but the CPU replacement point is outside the original PicoSoC structure. We need
SoC mode to replace the CPU inside PicoSoC while preserving existing SoC wiring.

## What Changes

1. Migrate only required PicoSoC source(s) into project-owned `src/top_soc` area.
2. Replace `picorv32` CPU instance in migrated `picosoc.v` with a Spike bus master.
3. Implement a DPI-backed `spike_bus_master` interface matching the original CPU
   memory bus contract.
4. Keep testbench responsibilities minimal: reset, memory preload, timeout, finish.
5. Generate firmware HEX with `riscv64-unknown-elf-elf2hex` when available,
   with explicit fallback behavior.
6. Route `build.sh run soc picorv32 <case>` through this new SoC integration path.

## Scope

In scope:
- `src/top_soc` SoC path refactor around migrated PicoSoC source.
- `build.sh` SoC build/run wiring and firmware HEX toolchain update.
- SoC regression for `hello`, `pico_test`, `mem`.

Out of scope:
- Changing CPU-mode cosim flows (`run cpu ...`).
- Broad vendor refactors unrelated to SoC path.

## Success Criteria

- SoC mode uses migrated PicoSoC with CPU slot replaced by Spike bus master.
- SoC testbench does not instantiate Spike directly.
- Firmware HEX path is deterministic and documented.
- `./build.sh run soc picorv32 all` passes.
- `./build.sh run cpu all hello` remains passing.
