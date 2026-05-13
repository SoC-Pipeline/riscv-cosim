## 1. OpenSpec Tracking

- [x] 1.1 Create proposal, design, specs, and tasks for shared Spike cosim logging.

## 2. Runtime Configuration

- [x] 2.1 Add explicit default PicoRV32 Spike commit log configuration in `build.sh`.
- [x] 2.2 Update the PicoRV32 VPI cosim init path to read runtime config for commit log and active firmware defaults.
- [x] 2.3 Remove stale PicoRV32 fallback references to `arith_basic_test`.

## 3. Documentation

- [x] 3.1 Update `README.md` to describe default per-target Spike commit logs.
- [x] 3.2 Update `docs/arch.md` to document the shared Spike build and wrapper split accurately.

## 4. Verification

- [x] 4.1 Run `bash -n build.sh`.
- [x] 4.2 Run a focused PicoRV32 cosim regression and confirm the default commit log is generated.
- [x] 4.3 Run a focused Ibex cosim regression and confirm the default commit log is generated.
- [x] 4.4 Review the generated log file layout and final OpenSpec task state.
