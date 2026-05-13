## Context

The supported regression is the active co-simulation path:

```text
firmware  -> build firmware artifacts under build/firmware/<TEST_NAME>/obj
vpi       -> build build/src/cosim/libspike.so and libspike.vpi
sim       -> build build/src/top/testbench.vvp and run vvp with Spike VPI
all       -> firmware, then vpi, then sim
clean     -> remove build/ and dump/
```

The current Makefile also includes older PicoRV32-native targets and toolchain download/build targets that are not part of the active regression. This change intentionally does not port those legacy targets.

## Goals

- Provide `build.sh` as the only supported active build entry point.
- Make default invocation user-friendly by printing help.
- Preserve the current active default case and environment override behavior.
- Remove `Makefile` after the current case passes through `build.sh`.

## Non-Goals

- Do not port inactive legacy Makefile targets such as `testbench_rvf.vvp`, `check.smt2`, `synth.v`, or toolchain builder recipes.
- Do not change firmware, RTL, VPI bridge, Spike, linker, or testbench behavior.
- Do not introduce a new build system dependency.

## Design

Implement `build.sh` as a Bash script with strict error handling:

```bash
#!/usr/bin/env bash
set -euo pipefail
```

The script resolves `TB_HOME` from its own location by default so it can be run from outside the repository root. Environment variables remain overridable:

```text
TEST_NAME
TB_HOME
SRC_DIR
SRC_TOP_DIR
COSIM_SRC_DIR
PICORV32_DIR
PICORV32_RTL
FIRMWARE_DIR
TEST_DIR
TEST_SRC_DIR
BUILD_DIR
DUMP_DIR
RISCV/RISCV_PATH/RISCV_TOOLCHAIN/RISCV_GNU_TOOLCHAIN_INSTALL_PREFIX/RISCV_ROOT
TOOLCHAIN_PREFIX
MY_ISA
MY_ELF_PATH
MY_HEX_PATH
MY_PK_PATH
PYTHON
IVERILOG
VVP
CXX
```

`help` should be the default command and should document the commands plus important override variables. Unknown commands should print help to stderr and return a non-zero status.

`all` should call `firmware`, `vpi`, and `sim` functions directly. `sim` should require the firmware HEX/ELF and VPI module to exist, then compile and run the simulation. This keeps direct `./build.sh sim` useful when artifacts already exist and gives a clear error otherwise.

`clean` should only remove paths that resolve under the repository root unless the user intentionally overrides `BUILD_DIR` or `DUMP_DIR` outside the repo. To avoid accidental broad deletion, it should reject empty paths and root paths before deletion.

## Verification

- Run `./build.sh` and confirm it prints help with exit code 0.
- Run `./build.sh clean`.
- Run `./build.sh all` with required modules loaded and confirm zero compare failures.
- Confirm active artifacts appear under `build/firmware` and `build/src`.
- Confirm `Makefile` has been removed after the passing run.
