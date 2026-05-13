## Why

The active co-simulation bridge is named and structured around Spike even though the project is expected to support additional RISC-V simulators later. This makes simulator-specific code, VPI plumbing, and comparison semantics harder to maintain or extend independently.

## What Changes

- Introduce a simulator-neutral C++ interface for the reference simulator behavior needed by the current co-simulation flow.
- Move Spike-specific object lifecycle, stepping, PC access, and instruction fetch behavior behind a Spike adapter.
- Keep the existing `$spike_*` VPI tasks and current Verilog behavior for compatibility while making the VPI layer call the neutral interface.
- Keep the first implementation focused on the active `src/cosim/spike_dpi.cc` path; do not activate or refactor `spike_dpi_thread.cc`.
- Update build script inputs and documentation only as needed for the new source files.

## Capabilities

### New Capabilities

- `cosim-simulator-interface`: Defines the neutral reference-simulator interface and the compatibility behavior of the active VPI bridge.

### Modified Capabilities

None.

## Impact

- Affects active files under `src/cosim`, especially `spike_dpi.cc`.
- May add new C++ source/header files under `src/cosim`.
- Affects `build.sh` if the VPI module needs to compile multiple source files.
- Affects `docs/arch.md` to describe the new VPI/frontend/adapter split.
- Preserves the current `./build.sh clean && ./build.sh all` regression behavior and compare result.
