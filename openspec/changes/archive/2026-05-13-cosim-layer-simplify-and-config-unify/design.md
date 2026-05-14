## Context

Active path:

```text
wrapper(VPI/DPI/C++ adapter)
  -> cosim_bridge
     -> CosimSession
        -> RiscvSimulator(SpikeSimulator)
```

Legacy path (unused in active build):

```text
spike_dpi_thread.cc
  -> sim.h / htif.h
```

## Design Decisions

### 1) Remove legacy threaded path

- Delete:
  - `src/cosim/spike_dpi_thread.cc`
  - `src/cosim/sim.h`
  - `src/cosim/htif.h`
- Rationale: not referenced by active build and duplicates backend integration logic.

### 2) Introduce shared config factory

Add one shared helper in `src/cosim` that builds `CosimConfig` from:
- `cpu_name`
- `elf_path`
- optional defaults (memory base/size)
- environment variables

This helper is used by:
- `spike_dpi.cc`
- `tb_ibex_cosim.cc`
- `tb_veer_el2_cosim.cc`

### 3) Unified log naming with compatibility

Default policy:
- compare log: `dump/<cpu_name>_cosim_result.log`
- commit log: `dump/<cpu_name>_spike_commit.log`

Env precedence:
1. new generic overrides (`COSIM_LOG_PATH`, `SPIKE_COMMIT_LOG_PATH`)
2. existing per-target variables (`PICORV32_*`, `IBEX_*`, `VEER_EL2_*`)
3. default `cpu_name` naming

### 4) Keep layer count, reduce duplication

Do not collapse `bridge/session/simulator` layers. They serve distinct responsibilities.
Instead:
- keep wrappers thin
- centralize config/log policy
- keep session semantics unchanged

## Validation Plan

After each step:
1. `./build.sh run picorv32 hello`
2. `./build.sh run ibex hello`
3. `./build.sh run veer_el2 hello`

Full regression at end:
1. `./build.sh run picorv32 pico_test`
2. `./build.sh run ibex pico_test`
3. `./build.sh run veer_el2 pico_test`
