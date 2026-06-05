# 架构

本文说明仓库内部如何组织和连接。命令用法、选择器、环境变量覆盖项和清理行为，请以 `./build.sh help` 与顶层 `README.md` 为准。

## 模式

本项目有两种执行模式，验证目的不同。

```text
                  CPU retire-compare mode
                  =======================

 firmware ELF/HEX
        |
        v
 target CPU testbench + DUT
        |
        | retire PC/instruction/events
        v
 cosim_bridge (DPI/C ABI)
        |
        v
 CosimSession
        |
        v
 SpikeSimulator
        |
        v
 compare counters + logs
```

```text
                  SoC Spike bus-driven mode
                  =========================

 firmware ELF -> firmware.hex
        |
        v
 TB $readmemh into PicoSoC RAM
        |
        v
 reset release
        |
        v
 Spike bus master replaces PicoRV32 CPU slot
        |
        | fetch/load/store bus transactions
        v
 PicoSoC RAM / UART-SPI regs / sim MMIO
        |
        v
 firmware PASS/FAIL/QUIT writes end simulation
```

CPU 退休比较模式在指令退休时用 Spike 校验 DUT CPU。SoC Spike 总线驱动模式将 DUT CPU 从执行路径中移除，用 Spike 驱动的总线主设备通过 PicoSoC 内存映射和外设执行固件。

## Spike / 仿真器角色

仓库以两种方式使用 Spike 后端仿真器。

### 1. CPU 开发：退休时指令比较

在 CPU 模式中，Spike 是黄金架构参考。DUT CPU 仍然执行固件，testbench/monitor 路径观测 DUT 已退休的内容，再将这些退休事件与 Spike 比较。

```text
DUT CPU executes firmware
  -> monitor captures retired architectural events
  -> CosimSession advances Spike
  -> DUT retire stream is compared against Spike retire stream
```

此模式用于验证 CPU 实现行为：退休指令流、GPR 写回、选定 CSR 行为，以及目标可用时的中断/调试同步界面。公共比较引擎是 `CosimSession`，但传入的退休侧信息量由各目标决定。

### 2. SoC 开发：仿真加速和 CPU 替换

在 SoC Spike 总线驱动模式中，Spike 不是并行黄金检查器，而是替代 PicoRV32 CPU 位置并直接驱动 SoC 总线的主动执行引擎。

```text
Spike executes firmware
  -> spike_bus_master issues fetch/load/store requests
  -> PicoSoC RAM and peripherals respond
  -> shared firmware/TB MMIO reports PASS/FAIL/FINISH
```

此模式用于验证 SoC 集成，并在不需要完整 RTL CPU 执行时加速软件 bring-up。核心问题是 SoC 内存映射、RAM 路径和外设可见行为能否支持经 SoC 总线驱动的固件正确执行。

## CPU 模式数据流

CPU 退休比较 testbench 位于 `src/top_cpu/`。每个目标都围绕同一套参考比较层提供目标相关 wrapper。

### 共享 Monitor 模型

虽然三个目标导出的信号集合不同，CPU 退休比较模式仍有共享 monitor 概念。公共路径如下：

```text
target retire observation
  -> target-specific DPI glue
  -> MonInstrTxn
  -> MonInstr
  -> CosimSession and/or target-specific checker
```

`MonInstrTxn` 是本地 monitor 路径使用的规范化数据包。字段类别包括：

- 退休指令身份：退休顺序、`pc`、`instr`
- trap 指示：一个退休级 trap 位
- GPR 写回：目标寄存器和写入值
- 内存副作用：地址、掩码、数据
- CSR 观测：地址、读掩码/数据、写掩码/数据

当前 CPU 退休比较 monitor 覆盖情况：

| 主题 | 共享 monitor 模型 | 当前实现状态 |
| --- | --- | --- |
| 退休指令 | `order`、`pc`、`instr` | PicoRV32、Ibex 和 VeeR EL2 已实现 |
| GPR 写回 | `gpr.addr`、`gpr.data` | PicoRV32 和 Ibex 已实现；VeeR EL2 当前不可信 |
| FPR 写回 | 当前无共享字段 | 未实现 |
| CSR 观测 | `csr.addr`、`csr.rmask/rdata/wmask/wdata` | PicoRV32 部分实现；VeeR EL2 未规范化；Ibex 的 CSR/debug/IRQ 权威处理仍在上游 checker 流程中 |
| 中断 / 异常 | 当前表示为退休级 `trap` | 已捕获 trap 位；中断源 / 异常原因尚未在 `MonInstrTxn` 中规范化 |
| 内存副作用 | `mem.addr`、掩码、数据 | PicoRV32 记录日志；尚未作为共享强制比较界面 |

因此，共享 monitor 路径已经覆盖核心退休流和部分架构副作用，但不会被视为所有目标功能等价。可信边界是目标相关捕获逻辑，而不是公共数据包类型本身。

### PicoRV32

```text
firmware.hex
  -> tb_picorv32.v memory preload
  -> PicoRV32 DUT executes firmware
  -> RVFI retire observation in tb_picorv32.v
  -> src/mon/mon_instr transaction logging
  -> DPI calls in src/top_cpu/tb_picorv32_cosim.cc
  -> CosimSession::step_detail()
  -> SpikeSimulator golden state
```

PicoRV32 使用 Verilator 顶层 testbench，并通过 `src/top_cpu/tb_picorv32_cosim.cc` 中的项目自有 DPI 回调接入。

PicoRV32 是第一个接入 monitor 路径的 CPU 目标。DUT 数据边界是启用 `RISCV_FORMAL` 后由 `picorv32_axi` 发出的 RVFI 退休接口；项目不会修改 `external/picorv32/picorv32.v` 来添加协同仿真插桩。每个已知的 `rvfi_valid` 退休都会通过 PicoRV32 专用 DPI 回调报告退休顺序、PC、指令、trap 标志、GPR 写回、RVFI 内存字段和受支持的 RVFI CSR 观测。monitor 将完整数据包写入 `PICORV32_MON_LOG`，并把 PC、GPR 写回以及可选 CSR 元数据提交给 `CosimSession::step_detail()`。

第一阶段 PicoRV32 monitor 检查覆盖退休 PC 和该指令的单个架构 GPR 写回。RVFI 内存字段会记录用于调试和后续扩展，但 `CosimSession` 尚未强制检查内存一致性。PicoRV32 还会暴露 `cycle`、`cycleh`、`instret`、`instreth` 的 RVFI 计数器 CSR 读观测；这些观测会被记录并传递到 monitor transaction，但被视为易变项，不会单独导致比较失败。FPR 和向量寄存器 monitor 资源仍留给支持 F/V 的目标后续扩展。

### Ibex

```text
firmware.elf
  -> Ibex simple-system RAM load
  -> ibex_top_tracing retire/checker signals
  -> project-local monitor transaction log
  -> upstream-style riscv_cosim_* DPI calls
  -> local compatibility wrapper
  -> CosimSession
```

Ibex 目标保持与上游 Ibex DPI checker 界面的兼容，同时适配到项目本地的 `CosimSession` 实现。本地 monitor 路径并行运行，从 RVFI 退休字段写出 `MonInstrTxn` 日志，包括退休指令身份、GPR 写回和退休级 trap 指示。现有 Ibex checker 仍是退休、中断/调试和 dside 同步的权威比较路径。

### VeeR EL2

```text
firmware.elf + firmware_veer.hex
  -> upstream VeeR EL2 tb_top memory load
  -> project monitor observes trace_rv_i_* retire signals
  -> MonInstrTxn log + retire compare
  -> Verilator DPI adapter
  -> CosimSession
```

项目 monitor 绑定到上游 VeeR testbench，不为协同仿真路径修改 vendor RTL。第一阶段 VeeR monitor 只使用公开 trace 退休接口。当前 monitor 数据包包括：

- 来自 `trace_rv_i_*` 的退休 PC/指令
- 来自 `trace_rv_i_exception_ip | trace_rv_i_interrupt_ip` 的 trap 指示

VeeR 当前在 `Retire` 模式中仅使用 `pc/instr` 进行比较。VeeR 当前不会通过共享 monitor 路径记录 GPR 或 CSR 写回，因为上游 `tb_top.wb_*` 镜像与退休指示在同一个时钟沿以非阻塞赋值更新，无法可靠对齐到正在退休的指令。后续扩展应从能保证与 `trace_rv_i_valid_ip` 周期对齐的 commit-stage 接口获取 GPR/CSR 数据。

## 协同仿真分层

```text
┌─────────────────────────────────────────────────────────────┐
│                    Testbench / Monitor                      │
│      PicoRV32 DPI       Ibex DPI        VeeR EL2 DPI        │
└──────────────┬──────────────┬──────────────┬────────────────┘
               │              │              │
               v              v              v
      ┌────────────────────────────────────────────┐
      │              cosim_bridge                  │
      │          C ABI wrappers for DPI users      │
      └────────────────────┬───────────────────────┘
                           v
      ┌────────────────────────────────────────────┐
      │              CosimSession                  │
      │ retire comparison, CSR/debug/IRQ sync, logs│
      └────────────────────┬───────────────────────┘
                           v
      ┌────────────────────────────────────────────┐
      │             SpikeSimulator                 │
      │       processor_t backend and simif_t      │
      └────────────────────────────────────────────┘
```

各层职责：

- `cosim_bridge`：面向 DPI 用户的 C ABI 边界，应保持轻量。
- `CosimSession`：验证策略、退休比较、计数器和日志生成。
- `RiscvSimulator`：后端无关的参考仿真器接口。
- `SpikeSimulator`：当前使用 Spike `processor_t` 的后端实现。
- `SimulatorCapabilities`：当后端无法支持某个同步界面时使用的特性标志。

当前默认后端是 Spike。新增后端主要需要添加一个新的 `RiscvSimulator` 子类，并在 `src/cosim/simulator_factory.cc` 中注册工厂。

## SoC 模式结构

SoC Spike 总线驱动模式基于项目自有的 PicoSoC 副本 `src/top_soc/picosoc.v`。上游 SoC 结构有意保持可识别：RAM、SPI flash 接口、UART 寄存器和外部 IO 路径仍然保留，原 PicoRV32 CPU 实例则替换为 `spike_bus_master`。

```text
                       src/top_soc/tb_picosoc_soc.sv
                                      |
                                      | $readmemh firmware.hex
                                      v
┌────────────────────────────────────────────────────────────────┐
│                         picosoc.v                              │
│                                                                │
│  ┌──────────────────┐       ┌────────────────────────────────┐ │
│  │ spike_bus_master │──────▶│ PicoSoC bus decode             │ │
│  └────────┬─────────┘       │ - RAM at 0x80000000            │ │
│           │                 │ - SPI/UART regs at 0x02000000  │ │
│           │                 │ - sim MMIO at 0x10000000       │ │
│           │                 └───────────────┬────────────────┘ │
│           │                                 │                  │
│           v                                 v                  │
│  ┌──────────────────┐             ┌──────────────────────────┐ │
│  │ spike_bus_dpi.cc │             │ picosoc_mem / devices    │ │
│  └──────────────────┘             └──────────────────────────┘ │
└────────────────────────────────────────────────────────────────┘
```

总线角色：

- Master：Spike，通过 `spike_bus_master.sv` 和 `spike_bus_dpi.cc` 接入。
- Slave：PicoSoC RAM、PicoSoC SPI/UART 寄存器，以及 TB 处理的仿真 MMIO。

本仓库使用的 SoC 地址映射：

```text
0x80000000... RAM
0x02000000... PicoSoC SPI/UART register region
0x10000000    shared firmware/TB simulation MMIO
```

仿真 MMIO 地址在 SPI flash 路径之前显式解码。这样固件状态和控制台写入可通过 `iomem_valid/iomem_wstrb/iomem_addr/iomem_wdata` 被 `tb_picosoc_soc.sv` 观测到。

## 固件与 HEX 加载

固件用例是 `firmware/` 下的自检程序。共享运行时代码位于 `firmware/common`。

```text
firmware/common/start.S       reset 入口、栈设置、sim_start、main 跳转
firmware/common/sections.lds  RAM 起始地址和 reset vector 放置
firmware/common/print.c       通过 sim MMIO 输出控制台内容
firmware/common/sim.c         START/PASS/FAIL/FINISH/QUIT 协议 helper
firmware/common/firmware.mk   可复用固件构建规则
firmware/<case>/main.c        用例相关自检
```

默认共享用例是 `hello`、`pico_test` 和 `mem`。每个用例通过 `sim_pass()` 报告成功，通过 `sim_fail()` 报告失败。

### CPU 退休比较模式加载

CPU 退休比较模式使用相同固件产物，但每个目标按自己的 testbench 约定加载：

```text
PicoRV32: firmware.hex      -> tb_picorv32 memory array
Ibex:     firmware.elf      -> Verilator --meminit RAM flow
VeeR EL2: firmware_veer.hex -> upstream VeeR memory image flow
```

所有 CPU 目标使用相同固件镜像约定：固件链接到 `0x80000000` 处的 RAM，并将 `_start` / `reset_vec` 放在 `0x80000080`。DUT reset 连线由目标决定；例如 Ibex simple-system 构建传入 `BootAddr = RESET_VECTOR - 0x80`，因此 core 仍会从链接后的 reset 入口开始取指。

`build.sh` 还会通过 `build/firmware/<case>/obj/.build.stamp` 下的 stamp 文件跟踪固件源文件、公共运行时源文件、链接脚本和每个用例的 Makefile，因此固件源文件变化后会在下一次运行前自动重建。

### SoC Spike 模式加载

SoC Spike 总线驱动模式刻意只保留一个固件镜像加载者：SystemVerilog testbench。

```text
firmware.elf
  -> firmware.hex
  -> tb_picosoc_soc.sv $readmemh
  -> uut.memory.mem
  -> resetn release
  -> Spike PC set to ELF entry
  -> Spike fetch/load/store requests go through the SoC bus
```

`spike_bus_dpi.cc` 不会把 ELF 或 HEX 内容预加载到私有仿真器内存。它只用 ELF 入口点设置 Spike 初始 PC，并让 `simif_t::addr_to_mem()` 返回 `nullptr`，强制取指和数据访问都经过 DPI 总线桥。SoC TB 会打印 RAM 读写计数器，证明运行确实使用 PicoSoC RAM 路径，而不是隐藏的 Spike 内存。

## Firmware/TB 协议

固件通过一个共享 MMIO 地址与所有 CPU 和 SoC testbench 通信：

```text
SIM_MMIO_ADDR = 0x10000000
```

协议字节值定义在 `firmware/common/sim.h`：

```text
0x80 RISCV_START   firmware started
0x81 RISCV_FINISH  normal finish marker
0x82 RISCV_FAIL    self-check failed
0x83 RISCV_PASS    self-check passed
0x00 RISCV_QUIT    stop request after PASS/FAIL/FINISH
```

写入同一地址的可打印字节值会被视为控制台输出。

`sim_pass()`、`sim_fail()` 和 `sim_finish()` 会先写入状态标记，执行 I/O fence，然后写入 `RISCV_QUIT`。fence 对观测外部总线写入的 CPU monitor 很重要；它防止终止用的 `QUIT` 写入在前置状态标记尚不可见时先被观测到。

在 SoC Spike 总线驱动模式中，TB 在看到 MMIO 写握手时记录 PASS/FAIL/FINISH，然后等待随后的 `RISCV_QUIT` 写入，再调用 `spike_bus_finish()` 和 `$finish`。这样可以避免在状态 store 仍等待总线响应时停止 Spike worker。超时和 Spike 总线错误也使用同一 finish 路径，并返回非零退出码。

## 日志与证据

运行流程会写出三类证据：

```text
log/run_<target>_<case>.log              target stdout/stderr capture
log/<target>_cosim_result.log            retire compare result log
log/<target>_spike_commit.log            Spike commit log when enabled
log/picorv32_mon.log                     PicoRV32 RVFI monitor packet log
log/ibex_mon.log                         Ibex RVFI monitor packet log
log/veer_el2_mon.log                     VeeR EL2 monitor packet log
log/run_soc_spike_<case>.log             SoC Spike run stdout/stderr capture
log/soc_spike_<case>_commit.log          SoC Spike commit log
```

CPU 模式的比较日志和 commit 日志也可以用 `COSIM_LOG_PATH` 与 `SPIKE_COMMIT_LOG_PATH` 统一重定向；当 `build.sh` 或本地 wrapper 提供目标相关环境变量时，目标相关变量优先。实际使用中，CPU 目标提供 `PICORV32_*`、`IBEX_*` 和 `VEER_EL2_*` 覆盖项，用于比较、commit 和 monitor 日志；SoC Spike 模式提供 `SOC_SPIKE_COMMIT_LOG` 作为每次运行的 Spike commit 日志。

SoC Spike 模式会打印运行期 RAM 访问计数器：

```text
[SOC] ram_reads=<N> ram_writes=<N>
```

这些计数器在 SoC TB 内部从 `uut.mem_valid && uut.mem_ready && uut.ram_sel` 收集，因此可以直接检查 Spike 是否通过 PicoSoC RAM 取指和访问。

## 扩展系统时的注意事项

添加 CPU 目标时，将目标相关 tracing 和内存加载约定保留在 `src/top_cpu`，再通过现有 bridge 界面把退休事件适配到 `CosimSession`。

添加仿真器后端时，保持 wrapper 稳定，并在 `RiscvSimulator` 后面添加后端。需要明确设置 `SimulatorCapabilities`；`CosimSession` 会用这些标志判断指令取指比较、CSR 访问、中断同步和调试请求同步是否可用。

添加固件用例时，使用共享运行时 helper，并让用例自检。通过用例应调用 `sim_pass()`，失败用例应调用 `sim_fail()`。
