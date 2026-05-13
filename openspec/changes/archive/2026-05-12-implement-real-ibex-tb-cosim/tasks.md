## 1. OpenSpec Tracking

- [x] 1.1 Create proposal, design, specs, and tasks for the real Ibex testbench change.

## 2. Project Ibex Testbench

- [x] 2.1 Replace `src/top/tb_ibex.sv` marker with a real Ibex simple-system style top.
- [x] 2.2 Add configurable Ibex boot/reset address parameters defaulting to the PicoRV32 firmware map.
- [x] 2.3 Map RAM and simulator-control addresses so the shared firmware ELF can execute and terminate.

## 3. Cosim Build Support

- [x] 3.1 Add project-owned Ibex cosim bind file for `tb_ibex`.
- [x] 3.2 Add project-owned Verilator C++ harness for `tb_ibex`.
- [x] 3.3 Add project-owned FuseSoC core metadata for the `tb_ibex` cosim target.
- [x] 3.4 Update `build.sh sim ibex` to build `tb_ibex` and load the shared PicoRV32 firmware ELF.

## 4. Documentation

- [x] 4.1 Update `docs/arch.md` to describe the project-owned Ibex testbench flow.
- [x] 4.2 Update README/TODO notes for the shared ELF and reset-vector behavior.

## 5. Verification

- [x] 5.1 Run `bash -n build.sh`.
- [x] 5.2 Run `./build.sh clean && ./build.sh sim ibex`.
- [x] 5.3 Run `./build.sh sim picorv32` or the equivalent firmware/vpi/sim sequence after any clean.
- [x] 5.4 Review final diff and OpenSpec task status.
