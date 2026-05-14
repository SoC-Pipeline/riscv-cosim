# TODO

1. cosim的的形态探索
1. 现在cosim支持cpu的指令对比，还有一种soc模式，spike直接接管soc中的cpu

## 需求

1. 探索soc模式，以picorv32 soc为例子
1. external/picorv32/picosoc/icebreaker_tb.v 是相关tb
1. 探索使用spike接管 external/picorv32/picosoc/picosoc.v 中的 picorv32
1. 代码后续规划放到 src/top_soc

## 测试 

1. picorv32: ./build.sh run picorv32 hello
1. ./build.sh run all all

## tools

```bash
module load riscv-toolchain/master-v20251230 openEDA/verilator/v5.010
```

### 文档

1. 更新 docs/arch.md

## 注意

1. 小步修改频繁回归测试
2. 结束需要 review，禁止擅自 git commit
3. 禁止修改代码格式，仅修改项目需要的

