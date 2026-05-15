## 1. Shared Helpers

- [x] 1.1 Add a shared `top_ensure_parent_directory(...)` helper in `src/top_cpu/cosim_top_utils.*`.
- [x] 1.2 Add a retire-only monitor submission helper in `src/mon/mon_instr/*`.

## 2. Target Glue Cleanup

- [x] 2.1 Update PicoRV32 and VeeR EL2 DPI glue to use the shared parent-directory helper.
- [x] 2.2 Update VeeR EL2 monitor glue to use the shared retire-only monitor helper.
- [x] 2.3 Update Ibex local monitor glue to use the shared parent-directory helper and align with the new helper boundary.

## 3. Verification

- [x] 3.1 Run targeted syntax/build checks for the touched top-level CPU glue paths.
- [x] 3.2 Run representative PicoRV32, Ibex, and VeeR EL2 monitor-path regressions and confirm custom monitor log paths still work.
