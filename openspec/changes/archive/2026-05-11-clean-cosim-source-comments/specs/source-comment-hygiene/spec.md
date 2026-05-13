## ADDED Requirements

### Requirement: Active source comments are functional and English
Active co-simulation source files SHALL avoid personal-name markers and non-English explanatory comments. Necessary comments SHALL describe functionality or constraints in English.

#### Scenario: Active source is inspected
- **WHEN** a maintainer searches active co-simulation source files for personal markers or non-English comments
- **THEN** the active build path does not contain such markers in explanatory comments

### Requirement: Active build path excludes commented-out dead code
Active co-simulation source files SHALL not retain large commented-out code blocks or disabled debug code that is not part of the current behavior.

#### Scenario: Active source cleanup is reviewed
- **WHEN** a maintainer reviews `src/cosim/spike_dpi.cc` and `src/top/testbench.v`
- **THEN** obsolete commented-out implementations and debug snippets are removed while live behavior remains intact

### Requirement: Cleanup preserves co-simulation behavior
The cleanup SHALL preserve the existing firmware build, VPI bridge build, simulation launch, and PicoRV32/Spike comparison behavior.

#### Scenario: Regression is run after cleanup
- **WHEN** `make all` runs with the required tool modules loaded
- **THEN** the co-simulation completes with zero compare failures

### Requirement: Threaded Spike bridge experiment remains available
The cleanup SHALL preserve `src/cosim/spike_dpi_thread.cc` for now.

#### Scenario: Experimental bridge file is checked
- **WHEN** the cleanup is complete
- **THEN** `src/cosim/spike_dpi_thread.cc` still exists
