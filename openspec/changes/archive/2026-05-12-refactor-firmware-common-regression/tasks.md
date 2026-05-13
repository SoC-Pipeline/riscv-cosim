## 1. OpenSpec Tracking

- [x] 1.1 Create proposal, design, specs, and tasks for firmware common/refression change.

## 2. Firmware Runtime

- [x] 2.1 Add shared startup, linker, and print support under `firmware/common`.
- [x] 2.2 Convert `firmware/hello` to use common runtime and keep only case code.
- [x] 2.3 Convert `firmware/pico_test` to use common runtime and keep only case code.
- [x] 2.4 Remove active `arith_basic_test` build support from `build.sh`.

## 3. Build Regression

- [x] 3.1 Add explicit `FIRMWARE_CASES` defaulting to `hello pico_test`.
- [x] 3.2 Update generic firmware build to compile common sources plus selected case sources.
- [x] 3.3 Update `./build.sh all` to run every active case on `picorv32` and `ibex`.

## 4. Documentation

- [x] 4.1 Update `docs/arch.md` for common firmware runtime and all-regression behavior.
- [x] 4.2 Update README references that still describe `arith_basic_test` as default/active.

## 5. Verification

- [x] 5.1 Run `bash -n build.sh`.
- [x] 5.2 Run `./build.sh firmware`.
- [x] 5.3 Run `TEST_NAME=pico_test ./build.sh firmware`.
- [x] 5.4 Run `./build.sh sim`.
- [x] 5.5 Run `TEST_NAME=pico_test ./build.sh sim`.
- [x] 5.6 Run `./build.sh clean && ./build.sh all`.
- [x] 5.7 Review final diff and OpenSpec task status.
