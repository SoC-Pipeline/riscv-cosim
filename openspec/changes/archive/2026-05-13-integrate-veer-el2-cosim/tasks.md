## 1. Change Setup

- [x] 1.1 Normalize the VeeR top filename to `src/top/tb_veer_el2.sv` and remove stale `tb_el2_veer` tracking.
- [x] 1.2 Confirm VeeR EL2 source, wrapper, generated config inputs, and upstream smoke flow are present.

## 2. Phase 1: VeeR Build Bring-Up

- [x] 2.1 Add `build.sh` variables and helpers for VeeR EL2 paths under `build/src/top/veer_el2`.
- [x] 2.2 Generate VeeR EL2 config files from `external/Cores-VeeR-EL2/configs/veer.config` with `RESET_VECTOR`.
- [x] 2.3 Add `build -t veer_el2` support and keep `veer_el2` out of default `all`.

## 3. Phase 1: Project Top and Firmware Run

- [x] 3.1 Implement `src/top/tb_veer_el2.sv` as the project VeeR EL2 integration layer.
- [x] 3.2 Load project firmware artifacts from `build/firmware/<case>/obj`.
- [x] 3.3 Decode project print and finish addresses in the VeeR EL2 testbench.
- [x] 3.4 Validate `./build.sh run veer_el2 hello`.
- [x] 3.5 Validate `./build.sh run veer_el2 pico_test`.

## 4. Phase 2: Cosim Integration

- [x] 4.1 Initialize Spike cosim with the selected firmware ELF.
- [x] 4.2 Feed VeeR retire trace PC/instruction events into cosim.
- [x] 4.3 Finish cosim and report pass/fail status at simulation exit.

## 5. Verification and Documentation

- [x] 5.1 Re-run `./build.sh run picorv32` and `./build.sh clean && ./build.sh run ibex`.
- [x] 5.2 Update `docs/arch.md` with the staged VeeR EL2 integration.
