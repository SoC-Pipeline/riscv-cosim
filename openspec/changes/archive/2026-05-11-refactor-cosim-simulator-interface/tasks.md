## 1. OpenSpec Tracking

- [x] 1.1 Create proposal, design, spec, and tasks for the simulator interface refactor.

## 2. Interface Extraction

- [x] 2.1 Add neutral simulator interface and configuration/result types under `src/cosim`.
- [x] 2.2 Move ELF entry helper code out of the VPI task implementation.
- [x] 2.3 Add a Spike backend that owns Spike `cfg_t`, `sim_t`, and `processor_t` state.
- [x] 2.4 Update the VPI entrypoint to own a `RiscvSimulator` instance and forward existing `$spike_*` tasks through it.
- [x] 2.5 Preserve existing `$spike_*` task names and current `libspike` module name.

## 3. Build and Documentation

- [x] 3.1 Update `build.sh` to compile all active cosim source files into the VPI module.
- [x] 3.2 Update `docs/arch.md` to describe VPI frontend, simulator interface, and Spike backend responsibilities.

## 4. Verification

- [x] 4.1 Run `bash -n build.sh`.
- [x] 4.2 Run `./build.sh vpi` with required modules loaded.
- [x] 4.3 Run `./build.sh sim` after existing artifacts are available.
- [x] 4.4 Run `./build.sh clean && ./build.sh all` with required modules loaded and confirm zero compare failures.
- [x] 4.5 Review OpenSpec status and final diff.
