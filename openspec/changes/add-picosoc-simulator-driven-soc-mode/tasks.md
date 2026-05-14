## 1. OpenSpec tracking

- [x] 1.1 Create change proposal/design/tasks for PicoSoC simulator-driven SoC mode.

## 2. SoC shell (phase 1)

- [x] 2.1 Add `src/top_soc/picosoc_shell.sv` with external master interface (no CPU instance).
- [x] 2.2 Keep PicoSoC-style RAM/peripheral decode in shell and wire memory path.

## 3. Minimal testbench (phase 1)

- [x] 3.1 Add `src/top_soc/tb_picosoc_shell_mem.sv` that drives memory write/read via external master signals.
- [x] 3.2 Validate readback checks and clean pass/fail finish behavior.

## 4. Build integration (phase 1)

- [x] 4.1 Add a lightweight build entry for the SoC shell memory test (without touching existing run matrix).
- [x] 4.2 Run the new test and confirm pass.

## 5. Regression safety

- [x] 5.1 Run `./build.sh run picorv32 hello`.
- [x] 5.2 Run `./build.sh run all all`.

## 6. Simulator-driven SoC execution (phase 2)

- [x] 6.1 Add a bus-oriented SoC top (`tb_picosoc_shell_bus.sv`) exposing external master handshake signals.
- [x] 6.2 Add a Spike-driven harness (`tb_picosoc_shell_spike.cc`) to preload ELF into SoC RAM and execute through bus callbacks.
- [x] 6.3 Add `./build.sh soc-spike [case]` build/run entrypoint.
- [x] 6.4 Run `./build.sh soc-spike hello` and confirm finish marker is observed.

## 7. SoC execution usability (phase 3)

- [x] 7.1 Add visible SoC-mode UART character printing in Spike-driven harness.
- [x] 7.2 Extend SoC-mode runner to support case selector `all`.
- [x] 7.3 Run `./build.sh soc-spike all` and confirm `hello` and `pico_test` pass.
