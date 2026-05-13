## Context

The active co-simulation path builds `src/cosim/spike_dpi.cc` into `scripts/libspike.vpi`, compiles `src/top/testbench.v`, and runs firmware from `firmware/arith_basic_test`. These files still contain comments copied from experiments, personal initials, non-English explanatory text, and disabled code blocks.

`src/cosim/spike_dpi_thread.cc` is intentionally preserved in this change. It is not part of the active `make all` path and will not be deleted.

## Goals / Non-Goals

**Goals:**
- Clean the active co-simulation sources without changing behavior.
- Replace personal-name comments with functional descriptions.
- Remove commented-out dead code from the active build path.
- Keep necessary comments English and focused on non-obvious behavior.
- Run `make all` after cleanup.

**Non-Goals:**
- Do not delete `src/cosim/spike_dpi_thread.cc`.
- Do not rewrite the VPI bridge architecture.
- Do not reformat upstream PicoRV32 RTL broadly.
- Do not hand-edit large generated firmware streams beyond clear local dead-code/comment cleanup.

## Decisions

- Focus first on files used by `make all`: `src/cosim/spike_dpi.cc`, `src/cosim/sim.h`, `src/cosim/htif.h`, `src/top/testbench.v`, and small firmware support files.
- Remove the block-commented alternate `$spike_init` implementation from `spike_dpi.cc` because it is inactive, contains stale paths, and duplicates live logic.
- Keep `spike_dpi_thread.cc` unchanged unless a later change decides to make it active or archive it.
- Avoid sweeping changes in generated assembly tests; generated comments can be meaningful test provenance.

## Risks / Trade-offs

- Removing comments can hide intent -> keep short English comments where behavior is non-obvious.
- Dead-code removal can accidentally remove dormant debug hooks -> verify with `make all` and review remaining searches.
- Firmware assembly comments may be generated metadata -> defer broad cleanup there to avoid changing test provenance.
