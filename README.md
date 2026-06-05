# RISC-V 协同仿真

本仓库用于构建小型 RISC-V 固件用例，并运行两类验证流程：

- 面向 `picorv32`、`ibex`、`veer_el2` 的 CPU 退休指令比较模式
- 面向 `soc/picorv32` 的 SoC Spike 总线驱动模式，其中 Spike 替代 PicoRV32 CPU 位置，并通过 SoC 总线访问 PicoSoC 设备

## 仓库布局

```text
build.sh        构建/运行入口。使用 ./build.sh help 查看命令矩阵。
src/top_cpu/    CPU 退休比较测试平台和目标相关 DPI 适配器。
src/top_soc/    SoC Spike 总线驱动模式、Spike 总线主设备和 SoC 测试平台。
src/cosim/      共享 CPU 比较桥、CosimSession 和 Spike 后端。
firmware/       自检固件用例及共享启动/运行时代码。
scripts/        构建期辅助脚本。
external/       第三方依赖：PicoRV32、Ibex、Spike、pk、VeeR EL2。
build/          生成的固件、依赖构建结果和仿真器输出。
log/            运行日志、比较日志和 Spike commit 日志。
docs/arch.md    架构、数据流、固件加载和协议细节。
```

```text
                 +-----------------------------+
                 |        firmware/*           |
                 | hello / pico_test / mem     |
                 +--------------+--------------+
                                |
                                v
                      +---------+---------+
                      |       build.sh    |
                      | build / run / all |
                      +----+---------+----+
                           |         |
          +----------------+         +----------------+
          v                                       v
+---------+----------------------+    +------------+-------------+
| CPU Retire-Compare Targets     |    | SoC Spike Mode            |
| picorv32 / ibex / veer_el2     |    | PicoSoC + Spike bus       |
+---------+----------------------+    +------------+-------------+
          |                                        |
          v                                        v
+---------+----------------------+    +------------+-------------+
| retire trace -> CosimSession   |    | HEX preload -> SoC RAM    |
| CosimSession -> Spike backend  |    | Spike fetch/load/store    |
+---------+----------------------+    +------------+-------------+
          |                                        |
          +-------------------+--------------------+
                              v
                    +---------+---------+
                    |        log/       |
                    | run logs + commits|
                    +-------------------+
```

## 前置条件

常用开发环境需要 RISC-V GCC 工具链、`elf2hex`、Verilator、FuseSoC、`pkg-config`、`g++`、`make`、`python3`，以及外部子模块。

```bash
module load riscv-toolchain/master-v20251230 openEDA/verilator/v5.010
git submodule update --init --recursive
```

## 使用方法

`build.sh` 是命令用法的唯一准确信息来源。先从以下命令开始：

```bash
./build.sh help
```

常用冒烟运行：

```bash
./build.sh all
```

典型的定向运行：

```bash
./build.sh run cpu picorv32 hello
./build.sh run soc picorv32 all
```

常用环境变量覆盖项也可通过 `./build.sh help` 查看。常见项包括：

```bash
export BUILD_JOBS=8
export RESET_VECTOR=0x80000080
export FIRMWARE_CASES="hello pico_test mem"
export COSIM_LOG_PATH=log/custom_cosim.log
export SPIKE_COMMIT_LOG_PATH=log/custom_spike_commit.log
```

`hello`、`pico_test` 和 `mem` 是 `./build.sh all` 当前共享支持的用例。

CPU 退休比较运行还支持目标相关覆盖项，例如：

- `PICORV32_COSIM_LOG`、`PICORV32_SPIKE_COMMIT_LOG`、`PICORV32_MON_LOG`
- `IBEX_COSIM_LOG`、`IBEX_SPIKE_COMMIT_LOG`、`IBEX_MON_LOG`
- `VEER_EL2_COSIM_LOG`、`VEER_EL2_SPIKE_COMMIT_LOG`、`VEER_EL2_MON_LOG`

SoC Spike 模式使用 `SOC_SPIKE_COMMIT_LOG` 作为 Spike commit 日志路径。

## 输出

生成产物位于 `build/`，日志位于 `log/`。

主要输出区域：

```text
build/firmware/<case>/obj/      固件 ELF/HEX 产物。
build/spike/                    本地 Spike 安装前缀。
build/pk/                       本地 proxy kernel 安装前缀。
build/spike-build/              本地 Spike 构建目录。
build/pk-build/                 本地 proxy kernel 构建目录。
build/src/top_cpu/              CPU 退休比较仿真器输出。
build/src/top_cpu/picorv32/     PicoRV32 Verilator 输出。
build/src/top_cpu/soc/          SoC Spike 模式 Verilator 输出。
log/                            运行日志、比较日志、监视器日志、commit 日志。
```

使用 `./build.sh clean` 清理普通生成输出，同时保留耗时较长的依赖构建结果。使用 `./build.sh clean-all` 删除整个 `build/` 目录和日志。

## 更多细节

参见 [docs/arch.md](docs/arch.md)，了解架构、CPU 退休比较 / SoC Spike 数据流、固件 HEX 加载和 firmware/TB 协议。

## 参考资料

1. [Ibex co-simulation documentation](https://ibex-core.readthedocs.io/en/latest/03_reference/cosim.html)
1. [spike-cosim](https://github.com/farukyld/spike-cosim)
1. [cosim-arch-checker](https://github.com/tenstorrent/cosim-arch-checker)
