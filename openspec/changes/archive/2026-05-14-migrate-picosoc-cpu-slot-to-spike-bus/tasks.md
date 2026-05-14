## 1. Change setup
- [x] 1.1 Create change artifacts (`proposal.md`, `design.md`, `tasks.md`)

## 2. Minimal PicoSoC migration
- [x] 2.1 Copy required upstream `picosoc.v` into `src/top_soc/`
- [x] 2.2 Keep non-migrated dependencies referenced from `external/` where possible
- [x] 2.3 Verify baseline build before CPU replacement

## 3. CPU slot replacement
- [x] 3.1 Add `spike_bus_master.sv` with PicoRV32-compatible memory-bus ports
- [x] 3.2 Add DPI backend `spike_bus_dpi.cc`
- [x] 3.3 Replace CPU instance in migrated `picosoc.v` with `spike_bus_master`

## 4. SoC testbench + run flow
- [x] 4.1 Add/adjust SoC TB to preload HEX, apply reset, monitor finish/timeout
- [x] 4.2 Remove legacy SoC C++-top path from `build.sh`
- [x] 4.3 Wire `build.sh run soc picorv32 <case>` to new path

## 5. Firmware HEX generation
- [x] 5.1 Add `elf2hex` detection and preferred conversion path
- [x] 5.2 Keep deterministic fallback and error reporting
- [x] 5.3 Validate produced HEX with SoC runs

## 6. Validation + docs
- [x] 6.1 Run `run soc picorv32 all`
- [x] 6.2 Run `run cpu all hello`
- [x] 6.3 Update README/docs command and architecture notes as needed

## 7. Firmware/TB interaction protocol
- [x] 7.1 Port the PicoRV32 verif firmware/TB status interface into `firmware/common`
- [x] 7.2 Update CPU and SoC testbenches to consume the shared print/status address
- [x] 7.3 Validate firmware/TB PASS/FAIL handling with SoC and CPU runs

## 8. Firmware case Makefiles and self-checks
- [x] 8.1 Add reusable firmware Makefile rules and per-case Makefiles
- [x] 8.2 Add `install` flow that publishes case artifacts to `build/firmware/<case>/obj`
- [x] 8.3 Update `build.sh build firmware` to call case Makefiles
- [x] 8.4 Add explicit self-checks with `sim_pass()` / `sim_fail()` in `hello`, `mem`, and `pico_test`
- [x] 8.5 Validate local case Makefiles and shared CPU/SoC run flows

## 9. Review fixes for firmware/TB protocol
- [x] 9.1 Make SoC simulation MMIO decode explicit and non-overlapping
- [x] 9.2 Delay SoC PASS/FAIL finish until the firmware store handshake has completed
- [x] 9.3 Update architecture docs for the shared `0x10000000` START/PASS/FAIL/QUIT protocol
- [x] 9.4 Re-run SoC firmware/TB protocol validation
