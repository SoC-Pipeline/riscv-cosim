# Repository Guidelines

## Project Structure & Module Organization

This repository validates RISC-V targets with CPU retire-compare and SoC Spike bus-driven flows. Shared co-simulation code lives in `src/cosim/`, CPU testbenches and DPI adapters in `src/top_cpu/`, SoC mode in `src/top_soc/`, and monitor logic in `src/mon/`. Firmware cases are under `firmware/<case>/`; shared startup and simulator protocol code is in `firmware/common/`. Helper scripts are in `scripts/`, architecture notes in `docs/`, dependencies in `external/`, outputs in `build/`, and logs in `log/`.

## Build, Test, and Development Commands

Use `./build.sh help` as reference.

- `./build.sh build spike`: build local Spike and proxy-kernel dependencies.
- `./build.sh build firmware hello`: build one firmware ELF/HEX case under `build/firmware/hello/obj/`.
- `./build.sh run cpu picorv32 hello`: run a focused CPU retire-compare smoke test.
- `./build.sh run soc picorv32 mem`: run PicoSoC Spike bus-driven mode for `mem`.
- `./build.sh all`: run all supported CPU/SoC targets across `hello`, `pico_test`, and `mem`.
- `./build.sh clean`: remove generated outputs and logs while preserving slow dependency builds.
- `./build.sh clean-all`: remove the `build/` tree and logs.

Common overrides include `BUILD_JOBS`, `RESET_VECTOR`, `FIRMWARE_CASES`, and log path variables.

## Coding Style & Naming Conventions

Follow the style of the file you edit. Current C++ uses four-space indentation; firmware C and SystemVerilog commonly use tabs. Keep C++ types/classes in PascalCase, functions and variables in snake_case, headers paired with matching `.cc` files, and target-specific files prefixed with the target or testbench name. Use short comments only for non-obvious protocol or hardware behavior.

## Testing Guidelines

The build script is the test harness. Use focused runs as smoke tests, then run `./build.sh all` when dependencies are available. Firmware tests should be self-checking and finish through `sim_pass()` or `sim_fail()`. Add new cases as `firmware/<case>/main.c` plus a `Makefile` using `firmware/common/firmware.mk`; include them in `FIRMWARE_CASES` when exercising the matrix. Inspect compare, Spike commit, and monitor logs for mismatches.

## Commit & Pull Request Guidelines

Recent history uses Conventional Commit prefixes such as `feat:`, `fix:`, `refactor:`, and `docs:`. Keep subjects imperative and scoped, for example `fix: align Ibex reset vector handling`. Pull requests should describe the affected target or flow, list commands run, mention overrides or tool versions, and link related issues or OpenSpec changes.

## Agent-Specific Instructions

Do not edit generated outputs in `build/`, `log/`, firmware `build/` directories, or vendored code under `external/` unless explicitly required. Prefer target-scoped changes and update `docs/arch.md` when architecture, data flow, firmware loading, or simulator protocol behavior changes.
