## 1. OpenSpec Tracking

- [x] 1.1 Create proposal, design, specs, and tasks for the Ibex cosim target.

## 2. Build Target Selection

- [x] 2.1 Refactor `./build.sh sim` so `picorv32` remains the default target.
- [x] 2.2 Add `./build.sh sim picorv32` as an explicit alias for the existing flow.
- [x] 2.3 Add `./build.sh sim ibex` dispatch and help text.

## 3. Ibex Firmware Variant

- [x] 3.1 Add Ibex firmware build outputs under `build/firmware/<TEST_NAME>/ibex`.
- [x] 3.2 Add Ibex linker/startup support for RAM at `0x00100000`, entry at `0x00100080`, and halt through `0x00020008`.
- [x] 3.3 Ensure the Ibex firmware still uses the existing firmware test inputs where practical.

## 4. Ibex Verilator Cosim Integration

- [x] 4.1 Validate required Ibex tools and Spike pkg-config packages before building.
- [x] 4.2 Build the Ibex simple-system cosim target through FuseSoC with outputs under project `build/`.
- [x] 4.3 Run the Ibex Verilator binary with `--meminit=ram,<ibex firmware elf>`.
- [x] 4.4 Wire `src/top/tb_ibx.sv` into the integration as the project-owned Ibex testbench entry point or wrapper.

## 5. Documentation

- [x] 5.1 Update `docs/arch.md` with PicoRV32 and Ibex simulation target descriptions.
- [x] 5.2 Update user-facing build documentation for `./build.sh sim ibex` and required tools.

## 6. Verification

- [x] 6.1 Run `bash -n build.sh`.
- [x] 6.2 Run `./build.sh sim picorv32`.
- [x] 6.3 Run `./build.sh sim ibex`.
- [x] 6.4 Run `./build.sh clean && ./build.sh sim ibex`.
- [x] 6.5 Review final diff and OpenSpec task status.
