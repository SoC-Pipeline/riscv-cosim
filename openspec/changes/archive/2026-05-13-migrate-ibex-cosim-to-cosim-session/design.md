## Context

### Current State
The spike_cosim project supports three RISC-V targets:
- picorv32: Uses `CosimSession` → `SpikeSimulator` → `sim_t`
- veer_el2: Uses `CosimSession` → `SpikeSimulator` → `sim_t`
- ibex: Uses `SpikeCosim` (from external/ibex) directly creating `processor_t`

### Problem
The ibex cosim implementation (`SpikeCosim`) is architecturally different from the unified CosimSession approach:
1. Creates `processor_t` directly without `sim_t` wrapper
2. Uses `Cosim` base class interface with detailed step parameters
3. Includes ibex-specific CSR, PMP, and debug mode handling

### Constraints
- Must not break existing picorv32 and veer_el2 regressions
- Commit log output must remain identical
- Phase-based approach to minimize risk

## Goals / Non-Goals

**Goals:**
- Migrate ibex to use CosimSession as unified cosim layer
- Ensure all three targets produce identical commit logs
- Preserve all existing verification capabilities
- Small, incremental steps with regression testing at each phase

**Non-Goals:**
- Not rewriting SpikeCosim logic from scratch - porting existing code
- Not changing the Cosim interface to CosimSession interface
- Not modifying external/ibex - only using its `SpikeCosim` as reference

## Decisions

### Decision 1: Extend CosimSession vs Creating New Class

**Choice:** Extend `CosimSession` with optional detailed verification mode

**Rationale:** Creating a new class would duplicate CosimSession logic. Extending allows:
- ibex uses detailed mode (write_reg, sync_trap, suppress, CSR ops)
- picorv32/veer_el2 use simple mode (pc, instr only)
- Single code path for both modes

**Alternative:** Create `CosimSessionIbex` subclass
- Rejected: Would create parallel maintenance burden

### Decision 2: Phase 1 - Add CSR Support to SpikeSimulator

**Approach:** First extend `SpikeSimulator` to expose CSR operations:
- `set_csr(unsigned csr_num, uint32_t value)`
- `get_csr(unsigned csr_num)`
- `set_mip(uint32_t mip)`
- `set_nmi(bool nmi)`
- `set_debug_req(bool debug_req)`

**Rationale:** `SpikeCosim` uses these to track ibex state. `SpikeSimulator` needs them to provide similar functionality.

### Decision 3: Phase 2 - Extend CosimSession Interface

**Approach:** Add `step_detail()` method to `CosimSession`:
```cpp
void step_detail(uint32_t write_reg, uint32_t write_reg_data, uint32_t pc,
                bool sync_trap, bool suppress_reg_write);
```

**Rationale:** Maintains backward compatibility - existing `retire()` continues to work for picorv32/veer_el2.

### Decision 4: Phase 3 - Port Verification Logic

**Approach:** Port key functions from `SpikeCosim`:
1. `check_retired_instr()` - verify instruction retirement
2. `check_gpr_write()` - verify register writes
3. `handle_csr_ops()` - track CSR state changes
4. `notify_dside_access()` - memory access verification

## Risks / Trade-offs

| Risk | Mitigation |
|------|------------|
| Breaking picorv32/veer_el2 regressions | Phase-by-phase testing with `./build.sh run` after each phase |
| Commit log format divergence | Compare logs using `scripts/compare.py` after migration |
| CSR handling differences | Use `processor_t` CSR accessors from same Spike version |
| Performance regression | Benchmark before/after with `ibex_simple_system` |

## Migration Plan

### Phase 1: SpikeSimulator CSR Extension
1. Add CSR methods to `RiscvSimulator` base class
2. Implement in `SpikeSimulator::Impl`
3. Test with existing picorv32/veer_el2

### Phase 2: CosimSession Interface Extension
1. Add `step_detail()` method signature
2. Implement minimal version using existing `retire()` logic
3. Verify existing targets still work

### Phase 3: tb_ibex_cosim Migration
1. Replace `SpikeCosim` creation with `CosimSession`
2. Update DPI entry points to use new interface
3. Run ibex regression tests

### Phase 4: Verification Logic Porting
1. Port detailed check functions from SpikeCosim
2. Enable full ibex verification mode
3. Compare commit logs across all targets