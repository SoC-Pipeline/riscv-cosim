## Context

`build.sh` currently derives `SPIKE_ROOT` from `RISCV_ROOT`, so VPI compilation uses Spike headers and libraries from the loaded toolchain. It also searches for `pk` under `SPIKE_ROOT`, which works only while Spike and pk share the same external install prefix.

The repository now contains `external/riscv-isa-sim` and `external/riscv-pk`. `external/riscv-pk` is a healthy git repository with origin `git@github.com:riscv-software-src/riscv-pk.git`. `external/riscv-isa-sim` contains source files but has a stale `.git` pointer to a missing `.git/modules/riscv-isa-sim`; this change should still use the directory as the local source dependency and record intended metadata in `.gitmodules`.

## Goals

- Use local Spike headers and libraries for VPI builds.
- Use local pk for simulation.
- Keep generated dependency builds under `build/`.
- Preserve the RISC-V cross toolchain as the compiler/binutils provider.
- Keep `./build.sh all` as the single current-case regression command.

## Non-Goals

- Do not vendor or patch Spike/pk source contents in this change.
- Do not repair the stale git metadata inside `external/riscv-isa-sim` beyond declaring dependency metadata.
- Do not remove support for explicit override variables such as `SPIKE_ROOT`, `MY_PK_PATH`, or `TOOLCHAIN_PREFIX`.

## Design

Add dependency path variables:

```text
SPIKE_SRC_DIR  ?= $TB_HOME/external/riscv-isa-sim
SPIKE_BUILD_DIR ?= $BUILD_DIR/spike-build
SPIKE_PREFIX   ?= $BUILD_DIR/spike
PK_SRC_DIR     ?= $TB_HOME/external/riscv-pk
PK_BUILD_DIR   ?= $BUILD_DIR/pk-build
PK_PREFIX      ?= $BUILD_DIR/pk
BUILD_JOBS     ?= 8
```

Default `SPIKE_ROOT` to `SPIKE_PREFIX`. `SPIKE_INCLUDE_DIRS` and `SPIKE_LIB_DIR` should use `SPIKE_ROOT`.
The simulation command should prepend `SPIKE_LIB_DIR` to `LD_LIBRARY_PATH` so VPI runtime symbol resolution uses the same local Spike libraries that were used at link time, even when toolchain modules add another `libriscv.so` earlier in the environment.

Keep `RISCV_ROOT` and `TOOLCHAIN_PREFIX` responsible only for the RISC-V compiler/binutils. pk should be built with:

```text
--prefix=$PK_PREFIX
--host=riscv64-unknown-elf
--with-arch=rv32ic_zicsr_zifencei
```

The installed pk path should default to:

```text
$PK_PREFIX/riscv32-unknown-elf/bin/pk
```

Add commands:

```text
./build.sh spike  -> configure/make/install external/riscv-isa-sim into build/spike
./build.sh pk     -> configure/make/install external/riscv-pk into build/pk
./build.sh deps   -> spike + pk
./build.sh all    -> firmware + deps + vpi + sim
```

Use out-of-tree build directories to avoid writing generated files into dependency source directories. Spike and pk builds should run `make -j$BUILD_JOBS`, defaulting to 8 jobs to avoid unbounded parallel builds. The existing `./build.sh clean` can continue deleting `build/` and `dump/`, which removes dependency builds and installs.

## Risks

- Spike builds may require host packages such as device-tree compiler and Boost libraries.
- pk RV32 configuration must match the RV32IMC firmware flow. `rv32ic_zicsr_zifencei` matches pk README guidance for RVC-capable RV32 pk.
- Building dependencies during every clean all run increases runtime substantially.
- `external/riscv-isa-sim` has stale git metadata; future submodule cleanup may still be needed.

## Verification

- Run `./build.sh help`.
- Run `./build.sh clean && ./build.sh all` with required modules loaded.
- Confirm VPI links against `build/spike/lib`.
- Confirm VPI resolves `libriscv.so` from `build/spike/lib` at runtime.
- Confirm simulation plusargs use `build/pk/riscv32-unknown-elf/bin/pk`.
- Confirm zero compare failures.
