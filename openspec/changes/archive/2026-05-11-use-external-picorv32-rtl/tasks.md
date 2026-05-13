## 1. Dependency Metadata

- [x] 1.1 Add `.gitmodules` entry for `external/picorv32` with URL `git@github.com:YosysHQ/picorv32.git`.
- [x] 1.2 Ensure `external/picorv32` is represented as the PicoRV32 dependency and the placeholder `.gitkeep` is removed.

## 2. Build Path Migration

- [x] 2.1 Add Makefile variables for the external PicoRV32 directory and RTL file.
- [x] 2.2 Update the active `sim` build to compile `external/picorv32/picorv32.v`.
- [x] 2.3 Remove the tracked duplicate `src/top/picorv32.v`.

## 3. Documentation

- [x] 3.1 Update `docs/arch.md` to describe the external PicoRV32 RTL source and submodule checkout expectation.

## 4. Verification

- [x] 4.1 Confirm `src/top/picorv32.v` and `external/picorv32/picorv32.v` matched before removal.
- [x] 4.2 Run `make clean && make all` with the required modules loaded and confirm zero compare failures.
- [x] 4.3 Review OpenSpec status and final diff.
