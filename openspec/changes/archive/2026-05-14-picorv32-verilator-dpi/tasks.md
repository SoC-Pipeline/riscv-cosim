## 1. Testbench DPI Conversion

- [x] 1.1 Replace PicoRV32 `$cosim_*` VPI task usage in `tb_picorv32` with DPI-C imports and calls.
- [x] 1.2 Preserve the existing PicoRV32 retire deduplication, finish sequencing, and timeout behavior after the DPI conversion.

## 2. Verilator Cosim Harness

- [x] 2.1 Add a PicoRV32-specific Verilator C++ harness that implements the DPI entrypoints and reconstructs `MonInstrTxn` from the full RVFI retire payload.
- [x] 2.2 Add PicoRV32 Verilator runtime handling for time progression, optional tracing, finish cleanup, and cosim pass/fail exit status.

## 3. Build Flow Migration

- [x] 3.1 Replace the PicoRV32 CPU-mode build path in `build.sh` to compile `tb_picorv32` as a Verilator executable instead of an `iverilog` `.vvp`.
- [x] 3.2 Replace the PicoRV32 CPU-mode run path in `build.sh` to execute the Verilator binary directly and remove the PicoRV32 requirement for `libspike.vpi` and `vvp`.

## 4. Verification

- [x] 4.1 Run focused PicoRV32 CPU-mode build/simulation checks for representative firmware cases.
- [x] 4.2 Update the OpenSpec task checklist to reflect the implemented and verified migration state.
