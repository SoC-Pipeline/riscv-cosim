## 1. Phase A - Define unified bridge interface

- [x] 1.1 Inventory current entrypoints and map them to canonical lifecycle operations (`init`, `retire/step_detail`, `finish`, `destroy`, `status`)
- [x] 1.2 Define a single C ABI bridge contract and argument conventions
- [x] 1.3 Define compatibility alias strategy so existing entry names continue to work during migration
- [x] 1.4 Document error-handling and idempotency rules for bridge lifecycle

## 2. Phase B - Refactor wrappers to thin shims

- [x] 2.1 Refactor VPI wrapper to call canonical bridge functions only
- [x] 2.2 Refactor veer_el2 DPI wrapper to call canonical bridge functions only
- [x] 2.3 Refactor ibex C++ adapter path to align lifecycle and naming with canonical bridge
- [x] 2.4 Ensure wrapper logic does not duplicate verification behavior already in `CosimSession`

## 3. Phase C - picorv32 DPI prototype (keep VPI baseline)

- [x] 3.1 Add picorv32 DPI prototype path in parallel to existing VPI path
- [x] 3.2 Add build/run mode selector for picorv32 (`vpi` vs `dpi`)
- [x] 3.3 Validate prototype with baseline tests (`hello`, `pico_test`) and collect issues
- [x] 3.4 If DPI issues remain unresolved, keep both paths and record known limitations

## 4. Phase D - Cross-target verification and policy decision

- [x] 4.1 Run regression matrix: target × bridge mode
- [x] 4.2 Verify no regression for current default workflows
- [x] 4.3 Compare maintenance complexity and failure modes between paths
- [x] 4.4 Decide mode policy:
  - keep VPI default + DPI optional, or
  - switch default where stable

## 5. Documentation and handoff

- [x] 5.1 Update architecture docs with layered bridge model
- [x] 5.2 Document supported mode combinations and fallback guidance
- [x] 5.3 Add troubleshooting section for VPI/DPI lifecycle mismatches
