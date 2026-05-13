## 1. OpenSpec Tracking

- [x] 1.1 Create proposal, design, spec, and tasks for build CLI refactor.

## 2. Build CLI

- [x] 2.1 Update `build.sh` usage text for `build`, `run`, `all`, `clean`, and `clean-all`.
- [x] 2.2 Add argument parsing for `build spike`, `build -f [case|all]`, and `build -t [target|all]`.
- [x] 2.3 Add argument parsing for `run [target|all] [case|all]`.
- [x] 2.4 Remove old command entry points: `firmware`, `spike`, `pk`, `deps`, `vpi`, and `sim`.

## 3. Dependency Handling

- [x] 3.1 Add readiness checks for Spike/pk/VPI, firmware, PicoRV32 top, and Ibex top artifacts.
- [x] 3.2 Make `build spike` build Spike, pk, and VPI once and skip rebuilding when artifacts are present.
- [x] 3.3 Make firmware/top/run flows ensure required dependencies without rebuilding Spike/pk unnecessarily.
- [x] 3.4 Make Ibex default to `build/spike` and ensure required pkg-config files, including `riscv-fdt.pc`, are available there.

## 4. Clean Semantics

- [x] 4.1 Change `clean` to preserve Spike/pk build and install directories.
- [x] 4.2 Add `clean-all` to remove the full `build/` directory and `dump/`.

## 5. Documentation

- [x] 5.1 Update `README.md` for the new command model and clean behavior.
- [x] 5.2 Update `docs/arch.md` for build/run layering and local Ibex Spike dependency.

## 6. Verification

- [x] 6.1 Run `bash -n build.sh`.
- [x] 6.2 Run `./build.sh build spike`.
- [x] 6.3 Run `./build.sh clean` and verify `build/spike` and `build/pk` remain.
- [x] 6.4 Run `./build.sh build -f all`.
- [x] 6.5 Run `./build.sh build -t all`.
- [x] 6.6 Run `./build.sh run all all`.
- [x] 6.7 Run `./build.sh clean-all` behavior check, preserving any required follow-up rebuild validation if needed.
- [x] 6.8 Run `git diff --check` and review final OpenSpec task status.
