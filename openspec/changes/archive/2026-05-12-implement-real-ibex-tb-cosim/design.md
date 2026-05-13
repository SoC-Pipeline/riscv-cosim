## Context

The current Ibex path builds upstream `lowrisc:ibex:ibex_simple_system_cosim`
and runs `Vibex_simple_system`. That proves the external environment works, but
it leaves the project top as a marker instead of a real testbench. Upstream
cosim also hard-codes the top name, C++ generated type, RAM hierarchy, and Spike
start PC around `ibex_simple_system` and its `0x00100000` memory map.

The PicoRV32 firmware links at `0x80000000`. Ibex's first PC after reset is
`{boot_addr_i[31:8], 8'h80}`, so a shared ELF should place `reset_vec` at
`0x80000080` and set PicoRV32's reset PC to the same address. Ibex can then map
RAM at the PicoRV32 base, drive `boot_addr_i = 0x80000000`, and start the cosim
ISS at `0x80000080`.

## Goals / Non-Goals

**Goals:**

- Implement `src/top/tb_ibex.sv` as the real Verilator top for Ibex cosim.
- Make the Ibex boot address configurable and default it so the effective reset
  vector matches the PicoRV32 firmware entry.
- Build Ibex with the same `build/firmware/<TEST_NAME>/obj/firmware.elf` used
  by PicoRV32.
- Keep the upstream Ibex RTL and cosim sources unmodified.
- Continue using Verilator, FuseSoC, and the upstream Ibex DPI cosim checker.

**Non-Goals:**

- Do not merge the Ibex DPI cosim checker into the PicoRV32 VPI checker.
- Do not rewrite Ibex bus, timer, RAM, simulator control, or checker behavior
  beyond the address/top-name adaptation needed for the project testbench.
- Do not change the default PicoRV32 `./build.sh sim` behavior.

## Decisions

1. Use a project-owned SystemVerilog top named `tb_ibex`.

   The file should contain the same simple-system structure as upstream:
   `ibex_top_tracing`, `bus`, `ram_2p`, `simulator_ctrl`, and `timer`. Keeping
   instance names such as `u_top` and `u_ram` lets the cosim checker and C++
   harness remain simple.

2. Move the Ibex memory map to the PicoRV32 firmware range.

   Default `RamBaseAddr` is `32'h8000_0000`. The build uses a shared
  `RESET_VECTOR` value, defaulting to `0x80000080`, for PicoRV32's reset PC and
  derives Ibex `BootAddr` from the same value. The RAM size can remain 1 MB,
  which covers the existing 96 KB firmware region plus stack.

3. Add project-owned FuseSoC core metadata.

   A new core should reference `src/top/tb_ibex.sv`, the project bind file, and
   a project C++ harness while depending on upstream Ibex cores. This avoids
   modifying `external/ibex` and avoids relying on upstream core metadata that
   fixes the toplevel as `ibex_simple_system`.

4. Add a small project C++ harness for `tb_ibex`.

   The harness should mirror upstream `simple_system_cosim.cc`, but use
   `TOPLEVEL_NAME=tb_ibex`, generated `Vtb_ibex`, RAM hierarchy
   `TOP.tb_ibex.u_ram.u_ram`, Spike start PC `ResetVector`, and scope
   `TOP.tb_ibex`.

5. Keep one firmware build path.

   `./build.sh sim ibex` should run `firmware`, not `firmware ibex`, and pass
   `--meminit=ram,$MY_ELF_PATH`. The Ibex-specific startup and linker files can
   remain in the tree until a cleanup change removes them, but they should not
   be part of the active Ibex flow.

## Risks / Trade-offs

- [Risk] Upstream helper code assumes `ibex_simple_system` generated names.
  -> Mitigate by adding project harness files instead of trying to reuse the
  hard-coded upstream C++ classes directly.
- [Risk] The checker references internal Ibex hierarchy through `u_top`.
  -> Mitigate by keeping the `u_top` instance name and binding the checker to
  `tb_ibex`.
- [Risk] The PicoRV32 firmware writes to MMIO addresses that differ from the
  upstream Ibex simple-system controls.
  -> Mitigate by mapping simulator control at the address the shared firmware
  already uses or by adapting the testbench decode while keeping the ELF
  unchanged.
- [Risk] Verilator/FuseSoC generated output paths change with the new core name.
  -> Mitigate by deriving `IBEX_SIM_DIR` and `IBEX_SIM_EXE` from the project core
  name in `build.sh`.
