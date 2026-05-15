## Why

CPU mode is intended to validate core behavior, but the current PicoRV32 path only compares retired PC and instruction against Spike. PicoRV32 already exposes RVFI architectural retire data, so the flow can check deeper state without modifying the third-party core.

## What Changes

- Add a PicoRV32 CPU monitor path that consumes RVFI retire data and reports architectural state to the Spike co-simulation bridge.
- Migrate the instruction monitor concept from `tmp/cosim-arch-checker/mon/` into `src/mon/`, adapted to this repository's Spike/VPI flow instead of Whisper/CAC.
- Extend the PicoRV32 VPI co-simulation interface so CPU runs can compare retired PC plus GPR writeback data.
- Extend the monitor transaction model toward the original `monitor_gpr/fpr/vr/csr` shape by structuring GPR state and adding PicoRV32 CSR RVFI capture.
- Compare supported CSR read data through the Spike co-simulation bridge when the CSR is stable and log volatile counter CSR reads without claiming deterministic counter comparison.
- Write PicoRV32 monitor output to a dedicated log file.
- Update `docs/arch.md` to describe the CPU-mode monitor data flow and the first-stage check coverage.

## Capabilities

### New Capabilities
- `cpu-monitor-checks`: CPU-mode monitor checks that consume DUT retire data and perform deeper Spike lockstep comparison.

### Modified Capabilities
- None.

## Impact

- Affected code:
  - `src/top_cpu/tb_picorv32.v`
  - `src/cosim/spike_dpi.cc`
  - `src/cosim/cosim_bridge.*`
  - `src/cosim/cosim_session.*`
  - `src/mon/`
  - `build.sh`
  - `docs/arch.md`
- The first implementation targets PicoRV32 CPU mode only.
- The PicoRV32 path remains on the existing Icarus/VPI flow; no DPI dependency is introduced for PicoRV32.
- FPR and vector-register monitor APIs are design placeholders for future F/V-capable targets; PicoRV32 does not expose or implement those resources.
