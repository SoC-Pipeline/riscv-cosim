## Overview

The new command model separates artifact construction from execution:

```text
./build.sh build spike
  -> build/spike, build/pk, build/src/cosim/libspike.vpi

./build.sh build -f [case|all]
  -> build/firmware/<case>/obj/{firmware.elf,firmware.hex}

./build.sh build -t [target|all]
  -> build/src/top/tb_picorv32.vvp
  -> build/src/top/ibex/.../Vtb_ibex

./build.sh run [target|all] [case|all]
  -> ensure required artifacts
  -> execute the selected simulation matrix
```

The old `sim` command is removed. The `all` command remains as a regression
shortcut and should run the active matrix:

```text
for case in FIRMWARE_CASES:
  run picorv32 <case>
  run ibex <case>
```

## Dependency Boundaries

`build spike` owns slow external dependency artifacts:

- `build/spike-build`
- `build/spike`
- `build/pk-build`
- `build/pk`
- `build/src/cosim/libspike.so`
- `build/src/cosim/libspike.vpi`

Firmware builds require the local Spike/pk/VPI dependency set because the run
flow consumes the generated ELF/HEX with the local reference simulator. Top
builds also require the same dependency set.

Ibex shall use `build/spike` by default:

```text
IBEX_SPIKE_PREFIX = build/spike
IBEX_PKG_CONFIG_PATH = build/spike/lib/pkgconfig
```

The implementation must ensure the local Spike install exposes pkg-config files
needed by the Ibex cosim build, including `riscv-fdt.pc`. If upstream Spike
does not install that file by default in this environment, the build script
should install/copy it into `build/spike/lib/pkgconfig` from the Spike build
tree after `make install`.

The default Spike source is the Ibex-compatible fork under
`external/ibex/external/riscv-isa-sim`, installed into the project-owned
`build/spike` prefix. This keeps both supported simulation targets on one local
Spike install while satisfying the Ibex DPI checker API.

## Clean Semantics

`clean` is now a development clean:

```text
remove build/firmware
remove build/src
remove build/ccache
remove build/ccache-tmp
remove dump
preserve build/spike-build
preserve build/spike
preserve build/pk-build
preserve build/pk
```

`clean-all` is a full reset:

```text
remove build
remove dump
```

## CLI Parsing

The build command should support:

```text
./build.sh build spike
./build.sh build -f
./build.sh build -f hello
./build.sh build -f pico_test
./build.sh build -f all
./build.sh build -t
./build.sh build -t picorv32
./build.sh build -t ibex
./build.sh build -t all
```

Defaults:

- `build -f` means all active firmware cases.
- `build -t` means all supported targets.
- `run` with no arguments means all supported targets and all active firmware
  cases.
- `run <target>` means one target and all active firmware cases.
- `run <target> <case>` means one target and one case.

`all` can delegate to the same run matrix as `run all all`.

## Artifact Checks

Use explicit `require_*` helpers to decide whether a dependency is already
built. This keeps normal `clean` fast while allowing `run` to rebuild missing
firmware/top artifacts.

At minimum:

- Spike dependency readiness requires `build/spike/bin/spike`,
  Ibex cosim API headers in `build/spike/include/riscv/processor.h`,
  `build/pk/riscv32-unknown-elf/bin/pk`, and
  `build/src/cosim/libspike.vpi`.
- PicoRV32 top readiness requires `build/src/top/tb_picorv32.vvp`.
- Ibex top readiness requires `Vtb_ibex`.
- Firmware readiness requires the selected case ELF and HEX.

## Documentation

README and architecture docs should describe the new command model and the
changed clean behavior. References to `./build.sh sim` must be removed.
