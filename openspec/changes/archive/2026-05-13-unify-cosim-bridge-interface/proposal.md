## Why

Current co-sim entry paths are inconsistent even though all targets use Spike as ISS:

- `picorv32`: Verilog `$cosim_*` system tasks via VPI (`spike_dpi.cc`)
- `ibex`: C++ adapter path (`tb_ibex_cosim.cc`) forwarding to `CosimSession::step_detail`
- `veer_el2`: DPI-C functions (`tb_veer_el2.sv` + `tb_veer_el2_cosim.cc`)

This causes duplicated glue logic, diverging lifecycle handling, and higher maintenance cost when extending co-sim behavior.

The user requirement is:
1. Keep VPI support for picorv32 now
2. Prototype a DPI path for picorv32 and validate with `iverilog/vvp` flow constraints in mind
3. Prioritize interface-shape unification (not simulator/tool unification)
4. Keep fallbacks if DPI path has issues

## What Changes

This change introduces a unified co-sim bridge interface strategy in layers:

1. Define a single C-ABI co-sim bridge surface used by all frontends
2. Keep both VPI and DPI frontend shims where needed
3. Migrate ibex/veer/picorv32 entry glue to call the same C-ABI functions
4. Add a picorv32 DPI prototype path (parallel to existing VPI path)
5. Decide default/fallback policy based on validation results

## Scope Boundaries

In-scope:
- Co-sim bridge API and frontend shim alignment
- Build/test entry normalization for selecting VPI vs DPI path
- Documentation for supported modes and fallback behavior

Out-of-scope:
- Forcing a single simulator backend/toolchain
- Rewriting `CosimSession` verification semantics
- Removing VPI before DPI path is proven stable

## Impact

Expected benefits:
- One maintained API contract for Spike co-sim lifecycle
- Reduced duplicated glue logic across targets
- Easier onboarding of future RISC-V cores/simulators

Expected trade-offs:
- Temporary dual-path support (VPI + DPI) for picorv32
- Slightly more matrix testing during transition
