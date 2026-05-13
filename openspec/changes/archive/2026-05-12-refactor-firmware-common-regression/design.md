## Context

The active cosim firmware ABI is:

```text
RAM base      0x80000000
reset vector  0x80000080
print MMIO    0x10000000
finish MMIO   0x20000000, data 123456789
```

`hello` already follows this ABI. `pico_test` still links at `0x00000000`, has a
local startup file, local linker script, and local print helper. This prevents
it from running under the shared cosim flow.

## Decisions

1. Use `firmware/common` as the only active runtime support layer.

   Common owns:

   - `start.S`: reset entry, stack setup, call to `main`, finish MMIO write,
     then spin.
   - `sections.lds`: shared RAM base and reset vector placement.
   - `print.c` / `print.h`: character, string, decimal, and hex output helpers.
   - Optional `sim.h` or equivalent declarations if finish helpers become C
     callable later.

2. Keep case directories small.

   `firmware/hello` should contain only hello-specific C code. `firmware/pico_test`
   should contain only pico_test-specific C code. Case-local startup, linker,
   and print files should be removed from active cases.

3. Remove `arith_basic_test` from active flow.

   The working tree already removed its sources, and the current request
   confirms this is official. `build.sh` and docs should no longer advertise or
   special-case `arith_basic_test`.

4. Make firmware discovery explicit.

   `FIRMWARE_CASES` defaults to `hello pico_test`. This avoids accidentally
   treating helper directories such as `common` as tests. Users can override the
   list when needed.

5. Define `all` as regression.

   `./build.sh all` should build required deps/VPI once, then run every active
   case on PicoRV32 and Ibex. Each case run should set `TEST_NAME` for the
   duration of that run without relying on global mutable shell state.

## Risks

- Running Ibex for every case is slower than the previous `all` behavior, but
  that matches the regression requirement.
- `build.sh clean && ./build.sh all` requires Spike/pk/VPI dependencies to be
  rebuilt or present. Existing dependency behavior should be reused rather than
  redesigned in this change.
