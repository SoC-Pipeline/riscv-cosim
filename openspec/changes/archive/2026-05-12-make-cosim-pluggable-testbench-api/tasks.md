## 1. OpenSpec Tracking

- [x] 1.1 Create proposal, design, spec, and tasks for the pluggable cosim testbench API.

## 2. C++ Cosim Session

- [x] 2.1 Add a `CosimSession` class that owns the reference simulator, compare counters, and log file.
- [x] 2.2 Implement retire-event comparison and reference stepping in `CosimSession`.
- [x] 2.3 Add status accessors for pass/fail counts.

## 3. VPI Task API

- [x] 3.1 Replace active `$spike_*` task registration with `$cosim_init`, `$cosim_retire`, `$cosim_finish`, and `$cosim_get_status`.
- [x] 3.2 Update VPI task implementations to delegate to `CosimSession`.
- [x] 3.3 Remove old active `$spike_*` compatibility tasks.

## 4. PicoRV32 Example Integration

- [x] 4.1 Update `src/top/tb_picorv32.v` to initialize cosim with `$cosim_init`.
- [x] 4.2 Update DUT retire sampling to call `$cosim_retire` directly.
- [x] 4.3 Update trap handling to call `$cosim_finish` and remove Verilog-owned compare arrays.

## 5. Build and Documentation

- [x] 5.1 Update `build.sh` to compile any new cosim C++ source files.
- [x] 5.2 Update `docs/arch.md` to document the plugin-style `$cosim_*` interaction.

## 6. Verification

- [x] 6.1 Run `bash -n build.sh`.
- [x] 6.2 Run `./build.sh vpi` with required modules loaded.
- [x] 6.3 Run `./build.sh sim`.
- [x] 6.4 Run `./build.sh clean && ./build.sh all` and confirm zero compare failures.
- [x] 6.5 Review OpenSpec status and final diff.
