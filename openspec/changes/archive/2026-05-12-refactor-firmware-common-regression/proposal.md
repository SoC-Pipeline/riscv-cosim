## Why

`firmware/hello` and `firmware/pico_test` currently duplicate startup, linker,
and print support. `pico_test` still uses the old `0x00000000` firmware address
model, so it builds but does not run in the active PicoRV32/Ibex cosim flow.

The project also needs `./build.sh all` to mean a real regression over active
firmware cases and both supported testbenches.

## What Changes

- Extract shared startup, linker, print, and simulation finish support into
  `firmware/common`.
- Convert `hello` and `pico_test` into small case directories that use
  `firmware/common`.
- Officially remove `arith_basic_test` from the active firmware flow.
- Update `build.sh firmware` to compile common sources plus the selected case.
- Update `./build.sh all` to run active firmware cases on both `picorv32` and
  `ibex`.
- Update documentation to describe the common firmware layout and regression
  behavior.

## Impact

The active firmware cases become `hello` and `pico_test`. Both should link at
the shared `0x80000080` reset vector, print through `0x10000000`, and finish
through `0x20000000` in both supported testbench environments.
