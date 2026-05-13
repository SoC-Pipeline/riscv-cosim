## Why

The repository currently keeps an active PicoRV32 RTL copy under `src/top/` while also carrying the upstream PicoRV32 repository under `external/`. Using the external repository as the single RTL source avoids duplicate core copies and makes dependency provenance explicit.

## What Changes

- Build the active co-simulation against `external/picorv32/picorv32.v`.
- Register `external/picorv32` as a git submodule using the official upstream URL `git@github.com:YosysHQ/picorv32.git`.
- Remove the tracked `src/top/picorv32.v` copy after the build points at the external RTL.
- Update architecture documentation to describe the new PicoRV32 RTL source.
- Preserve the existing `make all` regression behavior.

## Capabilities

### New Capabilities
- `external-picorv32-rtl`: Tracks use of the external PicoRV32 upstream repository as the active RTL dependency.

### Modified Capabilities

## Impact

- Affects `Makefile`, `.gitmodules`, `docs/arch.md`, and PicoRV32 RTL source ownership.
- Removes `src/top/picorv32.v` from the project-owned source tree.
- Requires submodule-aware checkout/update behavior for `external/picorv32`.
