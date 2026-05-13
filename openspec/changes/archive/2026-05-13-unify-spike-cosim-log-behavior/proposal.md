## Why

The repository now builds one local Spike installation under `build/spike`, but
the active cosim targets do not expose a consistent default logging policy or a
fully aligned default firmware input. This makes it hard to compare Spike commit
logs across PicoRV32, Ibex, and VeeR EL2 and obscures whether a mismatch comes
from the DUT or from setup drift.

## What Changes

- Enable default Spike commit log output for PicoRV32, Ibex, and VeeR EL2.
- Standardize default log file locations for all active cosim targets.
- Align PicoRV32's default fallback firmware paths with the active `hello`
  firmware flow and shared reset vector layout.
- Ensure the build and documentation describe the shared-Spike model
  accurately: one local Spike build, multiple cosim wrappers.
- Remove stale PicoRV32 fallback references to deleted firmware cases.

## Capabilities

### New Capabilities
- `shared-spike-cosim-logging`: Default-on Spike commit logging and aligned
  default firmware/log conventions across PicoRV32, Ibex, and VeeR EL2 cosim
  targets.

### Modified Capabilities

## Impact

- Affected code: `build.sh`, `src/cosim/spike_dpi.cc`, `src/top/tb_picorv32.v`,
  `docs/arch.md`, and `README.md`
- Affected outputs: default files under `dump/` and `log/`
- No new external dependencies
