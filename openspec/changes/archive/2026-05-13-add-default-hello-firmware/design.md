## Context

The project now supports both PicoRV32 and Ibex using a shared firmware ELF
entry at `RESET_VECTOR`, defaulting to `0x80000080`. Both active testbenches
already decode firmware output writes at `0x10000000` and finish writes at
`0x20000000`.

PicoRV32 upstream's hello path is small: a C function prints `"hello world\n"`
through helper functions that store characters to `0x10000000`. The existing
repository has a `firmware/pico_test` print helper, but it links at address
`0x00000000` and is not the requested `firmware/hello` case.

## Decisions

1. Make `hello` the default test.

   `TEST_NAME` should default to `hello`. The default firmware artifacts become
   `build/firmware/hello/obj/firmware.elf` and `.hex`.

2. Add a generic firmware case layout.

   Cases with `start.S` and `sections.lds` should be built generically from all
   local `.S` and `.c` files in that firmware directory. This keeps
   `firmware/hello` self-contained and avoids hard-coding each future small C
   firmware case in `build.sh`.

3. Preserve the existing arithmetic build.

   `arith_basic_test` currently uses specific source names and linker behavior.
   It should stay available through the current build logic so existing
   regression/debug flows are not removed.

4. Use the existing shared MMIO ABI.

   `firmware/hello` should print by writing bytes to `0x10000000`. It should
   terminate by writing `123456789` to `0x20000000` and then spinning so the
   testbench controls simulation shutdown. This works for both PicoRV32 and
   Ibex.

5. Make PicoRV32 print output character-oriented.

   Ibex already writes UART output as a character stream. PicoRV32 should do the
   same for `0x10000000` writes so hello output appears as normal printed text.
   Verbose address/data diagnostics can remain behind the existing `verbose`
   switch.

## Risks

- Generic source discovery can accidentally include generated files if outputs
  are placed under the firmware source directory. Build outputs already live
  under `build/firmware`, so this is acceptable.
- Defaulting to hello changes `./build.sh sim` behavior. This is intentional per
  the current request; arithmetic remains available via `TEST_NAME=arith_basic_test`.
