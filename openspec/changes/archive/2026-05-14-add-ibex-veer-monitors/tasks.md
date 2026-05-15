## 1. Change setup

- [x] 1.1 Create change artifacts (`proposal.md`, `design.md`, `tasks.md`, and specs)

## 2. Shared monitor runtime

- [x] 2.1 Generalize `mon_instr` initialization so non-PicoRV32 targets can set explicit log paths
- [x] 2.2 Document target-specific monitor log outputs in architecture notes as needed

## 3. Ibex monitor integration

- [x] 3.1 Add a project-local Ibex monitor hook that builds `MonInstrTxn` from RVFI retire fields
- [x] 3.2 Keep the existing Ibex checker path authoritative while writing parallel monitor logs
- [x] 3.3 Validate Ibex build/run flow with the new monitor path enabled

## 4. VeeR EL2 first-stage monitor integration

- [x] 4.1 Replace the direct `veer_cosim_retire(pc, instr)` path with a shared monitor transaction path
- [x] 4.2 Report trace-based trap status in the VeeR monitor transaction and keep unavailable resources invalid
- [x] 4.3 Validate VeeR EL2 build/run flow with the new monitor path enabled

## 5. VeeR EL2 hierarchical follow-up

- [x] 5.1 Add hierarchical GPR monitor capture from stable `el2_dec` writeback signals
- [x] 5.2 Evaluate hierarchical CSR monitor capture and land it only if signal alignment is stable
- [x] 5.3 Update documentation on final VeeR monitor coverage and limitations
