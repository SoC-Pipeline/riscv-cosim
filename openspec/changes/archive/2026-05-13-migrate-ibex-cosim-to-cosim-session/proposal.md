## Why

Currently, the spike_cosim project has inconsistent architecture:
- picorv32 and veer_el2 use `CosimSession` → `SpikeSimulator` → `sim_t`
- ibex uses a separate `SpikeCosim` implementation from external/ibex

This architectural divergence means:
1. Different commit log formats across targets
2. Duplicated code and maintenance burden
3. Inconsistent behavior when running the same ELF on different targets

Migrating ibex to use CosimSession unifies all three targets (picorv32, ibex, veer_el2) to use the same Spike backend, ensuring identical commit log output when executing the same ELF.

## What Changes

- Extend `CosimSession` interface to support ibex detailed verification model
- Extend `SpikeSimulator` to expose CSR operation methods
- Migrate verification logic from `SpikeCosim` to `CosimSession`
- Update `tb_ibex_cosim.cc` to use `CosimSession` instead of `SpikeCosim`
- Ensure backward compatibility: existing tests for picorv32 and veer_el2 continue to pass

## Capabilities

### New Capabilities

- `cosim-session-ibex-mode`: Extension to CosimSession to support ibex-style step verification with write_reg, sync_trap, suppress_reg_write, CSR operations, and detailed error tracking
- `spike-simulator-csr`: Extension to SpikeSimulator exposing processor CSR read/write operations

### Modified Capabilities

- `cosim-session`: Add step_with_detail() method variant to support ibex verification needs while maintaining backward compatibility with existing retire() interface

## Impact

**Affected Code:**
- `src/cosim/cosim_session.cc` - add detailed step interface
- `src/cosim/spike_simulator.cc` - add CSR operations
- `src/cosim/riscv_simulator.h` - extend base class interface
- `src/top/tb_ibex_cosim.cc` - migrate to use CosimSession
- `external/ibex/dv/cosim/spike_cosim.cc` - reference for logic移植

**Dependencies:**
- All three targets (picorv32, ibex, veer_el2) must maintain regression test pass
- Commit log format must be identical across all targets