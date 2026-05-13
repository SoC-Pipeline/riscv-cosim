## Context

PicoRV32 uses the project VPI `$cosim_*` API and compares retire PC/instruction pairs against Spike. Ibex uses the upstream Ibex DPI checker and a project-owned Verilator top. VeeR EL2 has an upstream testbench under `external/Cores-VeeR-EL2/testbench`, but that environment uses its own `program.hex`, `0xD0580000` mailbox, generated config tree, and logs.

The project firmware convention is different: firmware is built once under `build/firmware/<case>/obj`, reset starts at `RESET_VECTOR` which defaults to `0x80000080`, characters are written to `0x10000000`, and simulation termination writes `123456789` to `0x20000000`.

## Goals / Non-Goals

**Goals:**

- Add `veer_el2` as a project top target.
- Run VeeR EL2 with existing project firmware cases.
- Keep generated VeeR configuration and Verilator output in `build/src/top/veer_el2`.
- Use VeeR EL2 retire trace signals as the cosim observation point.
- Preserve current PicoRV32 and Ibex behavior.

**Non-Goals:**

- Do not modify VeeR EL2 RTL formatting or vendor source unless required for integration.
- Do not replace Ibex's upstream cosim checker.
- Do not make `veer_el2` part of the default `all` matrix until the target is stable.

## Decisions

1. Use `tb_veer_el2.sv` as the project-owned top.

   The VeeR upstream `tb_top.sv` is useful as a reference but carries test-specific behavior. A project-owned top can align memory-mapped print/finish addresses with PicoRV32 and Ibex without changing common firmware.

2. Use VeeR trace output for the first cosim path.

   VeeR exposes `trace_rv_i_valid_ip`, `trace_rv_i_address_ip`, and `trace_rv_i_insn_ip`. Those map naturally to the existing lightweight compare API: retire PC plus instruction. Register/CSR-level checking can be added later if needed.

3. Generate VeeR config into `build/src/top/veer_el2`.

   VeeR requires generated `common_defines.vh`, `el2_pdef.vh`, and `el2_param.vh`. Keeping them under `build/` avoids polluting `external/Cores-VeeR-EL2/run_*` directories.

4. Implement in phases.

   Phase 1 brings up the VeeR build and project firmware runs without Spike cosim. Phase 2 wires Spike cosim after the VeeR firmware path is stable. Documentation and regression checks are updated around the stable phase boundaries.

## Risks / Trade-offs

- VeeR EL2 build complexity is higher than PicoRV32. Mitigation: reuse VeeR's generated config and file list, keep the first project top close to the upstream testbench structure.
- The current project VPI cosim path is Icarus-oriented. Mitigation: if direct Verilator VPI is not practical, add a small Verilator harness/DPI bridge for the same `CosimSession` semantics instead of changing firmware.
- VeeR reset/config generation may not accept every shared `RESET_VECTOR` value. Mitigation: set generated `reset_vec` explicitly and validate early in `build.sh`.
- VeeR may run slower than existing targets. Mitigation: keep `veer_el2` out of default `all` until runtime is acceptable.
