## 1. OpenSpec Tracking

- [x] 1.1 Create proposal, design, specs, and tasks for default hello firmware.

## 2. Firmware

- [x] 2.1 Add self-contained `firmware/hello` startup, linker, print helper, and hello C source.
- [x] 2.2 Make `hello` the default `TEST_NAME`.
- [x] 2.3 Add a generic firmware build path for `start.S` / `sections.lds` based cases.
- [x] 2.4 Keep `TEST_NAME=arith_basic_test` using the existing arithmetic build path.

## 3. Testbench Print Behavior

- [x] 3.1 Update PicoRV32 `0x10000000` writes to emit character-stream output by default.
- [x] 3.2 Confirm Ibex already handles `0x10000000` character output.

## 4. Documentation

- [x] 4.1 Update `docs/arch.md` for the hello default and generic firmware layout.

## 5. Verification

- [x] 5.1 Run `bash -n build.sh`.
- [x] 5.2 Run `./build.sh firmware`.
- [x] 5.3 Run `./build.sh sim` for default PicoRV32 hello.
- [x] 5.4 Run `./build.sh clean && ./build.sh sim ibex` for default Ibex hello.
- [x] 5.5 Run `TEST_NAME=arith_basic_test ./build.sh firmware`.
- [x] 5.6 Review final diff and OpenSpec task status.
