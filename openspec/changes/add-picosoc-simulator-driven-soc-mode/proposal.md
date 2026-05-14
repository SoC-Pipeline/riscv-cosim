## Why

Current cosim targets validate CPU execution by comparing retire information.
For SoC-mode acceleration, the CPU inside DUT should be removed from the active
simulation path and replaced by a simulator-driven bus master.

`TODO.md` asks to explore PicoSoC as the first SoC-mode target, with code
planned under `src/top_soc`.

## What Changes

- Add a new SoC-mode implementation track under `src/top_soc`.
- Introduce a PicoSoC shell that keeps memory/peripheral decode and removes the
  internal `picorv32` instance.
- Add a first runnable testbench focused on basic memory read/write behavior.
- Keep existing CPU-mode flows (`picorv32`, `ibex`, `veer_el2`) unchanged.

## Impact

- No regression risk to existing targets in phase 1.
- Establishes a clean integration point for simulator-driven bus execution in
  later phases.
