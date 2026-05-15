## 1. Monitor And Backend Interface

- [x] 1.1 Create `src/mon/mon_instr` monitor transaction code adapted from the external monitor concept without Whisper/CAC dependencies.
- [x] 1.2 Add a PicoRV32-compatible monitor/detail VPI task path that calls the existing Spike co-simulation bridge detail comparison.
- [x] 1.3 Add monitor logging support with a configurable PicoRV32 monitor log path.

## 2. PicoRV32 Integration

- [x] 2.1 Replace the PicoRV32 `$cosim_retire(pc, instr)` path with RVFI-driven monitor/detail reporting.
- [x] 2.2 Preserve duplicate-retire protection and finish handling when PicoRV32 firmware passes, fails, traps, or times out.
- [x] 2.3 Keep memory fields available in monitor logs without making memory comparison a required first-stage check.

## 3. Build Flow

- [x] 3.1 Add the monitor sources and include paths to the VPI build.
- [x] 3.2 Add PicoRV32 monitor log configuration to `build.sh` and ensure run directories are created.
- [x] 3.3 Keep existing `./build.sh run cpu picorv32 <case>` and `./build.sh run all all` command behavior.

## 4. Documentation

- [x] 4.1 Update `docs/arch.md` with the PicoRV32 RVFI-to-monitor-to-Spike CPU-mode data flow.
- [x] 4.2 Document first-stage check coverage as retired PC plus GPR writeback, with memory fields logged for later extension.

## 5. Verification

- [x] 5.1 Run `./build.sh run cpu picorv32 hello`.
- [x] 5.2 Run `./build.sh run all all`.
- [x] 5.3 Review changed files for formatting churn and unintended third-party RTL edits.

## 6. Resource Monitor Extension

- [x] 6.1 Structure `MonInstrTxn` around valid GPR, memory, and CSR resource observations while preserving the existing PicoRV32 monitor behavior.
- [x] 6.2 Add CSR observation fields and bridge/session comparison support for stable masked CSR reads.
- [x] 6.3 Keep PicoRV32 volatile counter CSR observations logged but excluded from mandatory compare.
- [x] 6.4 Document FPR/VR as future monitor resources rather than supported PicoRV32 checks.

## 7. PicoRV32 CSR Integration

- [x] 7.1 Wire PicoRV32 RVFI `mcycle` and `minstret` CSR signals through `tb_picorv32.v`.
- [x] 7.2 Extend `$cosim_monitor_retire` VPI argument handling for CSR observations without breaking existing run commands.
- [x] 7.3 Update PicoRV32 monitor log columns to include CSR observation details.

## 8. Extended Verification

- [x] 8.1 Run `bash -n build.sh` and `git diff --check`.
- [x] 8.2 Compile-check `src/mon/mon_instr/mon_instr.cc`.
- [x] 8.3 Run `./build.sh run cpu picorv32 hello`.
- [x] 8.4 Run `./build.sh all`.
- [x] 8.5 Validate OpenSpec and review for unintended third-party RTL edits.
