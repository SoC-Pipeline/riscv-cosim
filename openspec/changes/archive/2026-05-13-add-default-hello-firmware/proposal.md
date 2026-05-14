## Why

`TODO.md` now asks for a default hello firmware case based on PicoRV32's
printing mechanism. The current default firmware is `arith_basic_test`, and the
active firmware build path is tightly coupled to that test's file names. This
makes it awkward to add a small self-contained C firmware case and use it across
both PicoRV32 and Ibex cosim targets.

## What Changes

- Add a self-contained `firmware/hello` case with local startup, linker script,
  print helper, and hello C code.
- Make `hello` the default `TEST_NAME`.
- Add a generic firmware build path for cases that provide `start.S`,
  `sections.lds`, and C/assembly sources.
- Keep the existing `arith_basic_test` build path available through
  `TEST_NAME=arith_basic_test`.
- Align PicoRV32 and Ibex print handling around the shared `0x10000000` output
  address and `0x20000000` finish address.
- Update architecture documentation for the hello default and firmware case
  layout.

## Impact

Users can run the default hello case with `./build.sh sim` and
`./build.sh clean && ./build.sh sim ibex`. Existing arithmetic firmware remains
selectable by setting `TEST_NAME=arith_basic_test`.
