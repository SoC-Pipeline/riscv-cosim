## 1. Layout Migration

- [x] 1.1 Move tracked Verilog files from `top/` to `src/top/`.
- [x] 1.2 Move tracked firmware test inputs from `tests/` to `firmware/`.
- [x] 1.3 Move tracked C/C++ bridge files from `scripts/` to `src/cosim/` while leaving Python utilities in `scripts/`.

## 2. Build Path Synchronization

- [x] 2.1 Update `Makefile` directory variables, build commands, VPI build, simulation, and clean targets for the migrated layout.
- [x] 2.2 Update `env.sh` exported firmware paths for the migrated layout.
- [x] 2.3 Update Verilog testbench default ELF and HEX fallback paths.
- [x] 2.4 Search for stale active references to old `top/`, `tests/`, and C/C++ `scripts/` paths and fix them.

## 3. Documentation

- [x] 3.1 Add `docs/arch.md` describing the migrated layout and co-simulation flow.
- [x] 3.2 Update `README.md` with the required modules and current regression command.

## 4. Verification

- [x] 4.1 Run `make all` with the required modules loaded and confirm the co-simulation still passes.
- [x] 4.2 Review the final diff and OpenSpec status.
