## Why

The migrated `src/` and `firmware/` trees still contain personal markers, non-English comments, large commented-out code blocks, and debug leftovers. Cleaning these up makes the active co-simulation code easier to review and maintain without changing simulation behavior.

## What Changes

- Remove dead commented-out code from active co-simulation sources.
- Replace personal-name comments with functional descriptions.
- Convert remaining necessary comments in active paths to English.
- Keep comments only where they clarify non-obvious behavior.
- Preserve `spike_dpi_thread.cc` for now; do not delete it in this change.
- Keep generated or upstream-derived content conservative, especially large generated firmware assembly.
- Preserve `make all` behavior and PicoRV32/Spike compare results.

## Capabilities

### New Capabilities
- `source-comment-hygiene`: Defines maintainability expectations for comments, dead code, and personal markers in active co-simulation source paths.

### Modified Capabilities

## Impact

- Affects active source files under `src/cosim/`, `src/top/`, and small firmware files under `firmware/`.
- Does not intentionally change generated firmware behavior, VPI task behavior, linker layout, or Makefile build semantics.
- Verification remains the module-loaded `make all` regression.
