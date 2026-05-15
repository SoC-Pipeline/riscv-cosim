## Why

`src/top_cpu/` 里的 PicoRV32、Ibex、VeeR EL2 DPI glue 已经全面转到 DPI，但公共的路径处理、monitor 初始化、retire-only monitor 提交仍然散落在各个 target 文件里。现在重复还不算大，但已经足够让后续修复和扩展变成三处同步维护。

## What Changes

- 提取 `top_cpu` 侧共享的父目录创建 helper，统一 monitor/log 路径准备逻辑。
- 为 `mon_instr` 增加 retire-only 的公共提交 helper，减少 target glue 里手工构造半空 `MonInstrTxn` 的重复代码。
- 收敛 PicoRV32、Ibex、VeeR EL2 的 DPI/monitor glue 到这些公共 helper。
- 保持各 target 的 RTL 采样方式、Verilator 主循环、Ibex upstream checker 适配不变。

## Capabilities

### New Capabilities
- `top-cpu-dpi-glue`: Shared helper behavior for top-level CPU DPI glue and monitor submission.

### Modified Capabilities

## Impact

- Affected code:
  - `src/top_cpu/cosim_top_utils.*`
  - `src/mon/mon_instr/*`
  - `src/top_cpu/tb_picorv32_cosim.cc`
  - `src/top_cpu/tb_veer_el2_cosim.cc`
  - `src/top_cpu/tb_ibex_cosim.cc`
- No expected changes to target-specific retire signal capture or simulator control flow.
