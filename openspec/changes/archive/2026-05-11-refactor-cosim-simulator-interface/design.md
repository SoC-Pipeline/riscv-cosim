## Context

`src/cosim/spike_dpi.cc` currently combines several responsibilities:

- VPI task registration and argument conversion.
- ELF entry lookup.
- Spike configuration and simulator lifecycle.
- Spike stepping, PC reads, and instruction fetches.
- Error reporting and cleanup.

That structure works for a single Spike-backed flow, but it makes every future simulator addition touch the VPI layer and testbench-facing ABI. The first step should isolate the active Spike implementation behind a small interface while preserving the current `$spike_*` tasks and regression behavior.

## Goals

- Keep the existing active co-simulation behavior unchanged.
- Separate VPI plumbing from simulator-specific implementation.
- Make Spike one backend implementation of a neutral reference-simulator interface.
- Keep the interface small and grounded in current scoreboard needs: init, step, PC, instruction fetch, and optional register access.
- Keep `spike_dpi_thread.cc` out of this change.

## Non-Goals

- Do not add a second simulator backend in this change.
- Do not rename Verilog tasks to `$cosim_*` yet.
- Do not change the scoreboard algorithm in `src/top/testbench.v`.
- Do not solve the separate `external/riscv-isa-sim` gitlink/submodule issue.
- Do not refactor inactive copied Spike headers unless they are required by the active build.

## Design

Add a neutral interface in `src/cosim`:

```text
RiscvSimulator
  init(config)
  step(count)
  pc()
  instruction_at(pc)
  reg(index)
  name()
```

Supporting types:

```text
CosimConfig
  elf_path
  isa
  memory_base
  memory_size
```

`SpikeSimulator` implements the interface using local Spike headers and libraries. It owns the Spike `cfg_t`, `sim_t`, and `processor_t` pointers that are currently global in `spike_dpi.cc`.

`spike_dpi.cc` remains the VPI entrypoint for this change. It should:

- Parse VPI arguments and plusargs.
- Own one `std::unique_ptr<RiscvSimulator>`.
- Create `SpikeSimulator` as the default backend.
- Forward `$spike_init`, `$spike_run_steps`, `$spike_get_pc`, `$spike_get_instr`, and `$spike_get_reg` to the interface.
- Register cleanup callbacks.

The current task names remain compatibility names:

```text
$spike_init      -> default backend: SpikeSimulator
$spike_run_steps -> RiscvSimulator::step
$spike_get_pc    -> RiscvSimulator::pc
$spike_get_instr -> RiscvSimulator::instruction_at
$spike_get_reg   -> RiscvSimulator::reg
```

The build script should compile the new adapter/interface source files into the existing `build/src/cosim/libspike.so` module. The module name can remain `libspike` for compatibility during this change.

## Future Direction

A later change can introduce:

- `$cosim_*` VPI task aliases.
- `+COSIM_BACKEND=<name>` backend selection.
- A normalized retire-trace interface if a future simulator cannot provide arbitrary `instruction_at(pc)` fetches.
- A second backend implementation.

## Risks

- C++ object lifetime changes can break VPI cleanup if ownership is not explicit.
- Spike API usage must continue to use installed local Spike headers, not inactive local header copies.
- The interface should not overfit future simulators before a second backend exists.
- Existing staged changes in `src/cosim` should not be reverted or overwritten.

## Verification

- Run `bash -n build.sh`.
- Run `./build.sh vpi` with required modules loaded.
- Run `./build.sh sim` after existing firmware/dependency artifacts exist.
- Run `./build.sh clean && ./build.sh all` with required modules loaded.
- Confirm `COMPARE PASS NUM = 11416, COMPARE FAIL NUM = 0`.
- Confirm `docs/arch.md` describes the VPI/frontend/adapter split.
