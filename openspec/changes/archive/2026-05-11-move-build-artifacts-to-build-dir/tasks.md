## 1. Build Directory Variables

- [x] 1.1 Add Makefile variables for `build/`, firmware build outputs, source build outputs, VPI output, and simulation `.vvp` output.
- [x] 1.2 Split firmware input paths from firmware output paths so recipes read from `firmware/<TEST_NAME>/` and write to `build/firmware/<TEST_NAME>/obj/`.

## 2. Active Build Migration

- [x] 2.1 Update `build` and `build2` recipes to write firmware objects and images under `build/firmware`.
- [x] 2.2 Update `libspikeso` to write `libspike.so` and `libspike.vpi` under `build/src/cosim`.
- [x] 2.3 Update `sim` to write `testbench.vvp` under `build/src/top` and load VPI/firmware artifacts from `build/`.
- [x] 2.4 Update `clean` to remove active generated outputs from `build/` and `dump/`.

## 3. Defaults and Documentation

- [x] 3.1 Update `env.sh` generated firmware path exports.
- [x] 3.2 Update `src/top/testbench.v` fallback ELF and HEX paths.
- [x] 3.3 Update `README.md` and `docs/arch.md` generated-output documentation.

## 4. Verification

- [x] 4.1 Run `git diff --check`.
- [x] 4.2 Run `make clean && make all` with required modules loaded and confirm zero compare failures.
- [x] 4.3 Confirm active outputs are generated under `build/firmware` and `build/src`, and old active output paths are not regenerated.
- [x] 4.4 Review OpenSpec status and final diff.
