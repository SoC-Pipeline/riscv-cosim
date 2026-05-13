## Context

`make all` currently builds firmware, compiles `src/top/testbench.v` with `src/top/picorv32.v`, loads `scripts/libspike.vpi`, and runs the co-simulation. The external PicoRV32 repository is present at `external/picorv32`, and its `picorv32.v` currently matches the tracked `src/top/picorv32.v` copy byte-for-byte.

## Goals / Non-Goals

**Goals:**
- Make the active simulation compile PicoRV32 from `external/picorv32/picorv32.v`.
- Remove the duplicate project-owned `src/top/picorv32.v`.
- Record the external PicoRV32 dependency in `.gitmodules` with the official upstream URL.
- Keep `make clean && make all` passing.

**Non-Goals:**
- Do not change PicoRV32 RTL contents.
- Do not update PicoRV32 to a different commit in this change.
- Do not change firmware, Spike, or VPI behavior beyond the RTL source path.

## Decisions

- Add Makefile variables for the external PicoRV32 location, for example `PICORV32_DIR` and `PICORV32_RTL`, and use `PICORV32_RTL` in the active `sim` recipe.
- Register the submodule URL as `git@github.com:YosysHQ/picorv32.git` per user direction.
- Delete `src/top/picorv32.v` after the build no longer references it.
- Update `docs/arch.md` to show `external/picorv32/picorv32.v` as the active core RTL and `src/top/` as the project testbench area.

## Risks / Trade-offs

- Submodule checkout becomes required for a clean clone before `make all` can run.
- Some legacy Makefile targets still refer to root-local PicoRV32 paths; this change should at minimum fix the active `make all` path, and any remaining legacy target cleanup can be separate if needed.
- Removing the duplicate RTL reduces ambiguity but may break ad hoc scripts that hard-code `src/top/picorv32.v`.
