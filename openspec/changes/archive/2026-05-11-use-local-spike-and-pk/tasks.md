## 1. OpenSpec Tracking

- [x] 1.1 Create proposal, design, spec, and tasks for local Spike and pk dependencies.

## 2. Build Script

- [x] 2.1 Add local Spike source/build/install variables and `./build.sh spike`.
- [x] 2.2 Add local pk source/build/install variables and `./build.sh pk`.
- [x] 2.3 Add `./build.sh deps` and include local dependency builds in `./build.sh all`.
- [x] 2.4 Link VPI against `build/spike` by default.
- [x] 2.5 Use `build/pk/riscv32-unknown-elf/bin/pk` by default during simulation.
- [x] 2.6 Preserve override behavior for toolchain, Spike, and pk paths.
- [x] 2.7 Limit Spike and pk build parallelism with `BUILD_JOBS=8` by default.

## 3. Metadata and Documentation

- [x] 3.1 Update `.gitmodules` metadata for `external/riscv-isa-sim` and `external/riscv-pk`.
- [x] 3.2 Update `README.md`, `docs/arch.md`, and `TODO.md` for local Spike/pk dependency builds.

## 4. Verification

- [x] 4.1 Run `./build.sh help`.
- [x] 4.2 Run `git diff --check`.
- [x] 4.3 Run `./build.sh clean && ./build.sh all` with required modules loaded and confirm zero compare failures.
- [x] 4.4 Confirm Spike installs under `build/spike`, pk installs under `build/pk`, and VPI/sim consume those local paths.
- [x] 4.5 Review OpenSpec status and final diff.
