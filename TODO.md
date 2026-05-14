# TODO

1. cosim的的插入式形态实现

## 需求

1. soc模式将spike/simulator插入替代cpu，以picorv32 soc为例子
1. external/picorv32/picosoc/icebreaker_tb.v 是相关tb
1. 使用spike接管 external/picorv32/picosoc/picosoc.v 中的 picorv32/cpu

## 测试 

1. picorv32: ./build.sh run soc picorv32 mem
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

