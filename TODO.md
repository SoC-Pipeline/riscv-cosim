# TODO

1. 理解代码 tmp/cosim-arch-checker
1. 理解 cosim-arch-checker 的monitor功能:
    - tmp/cosim-arch-checker/README.md
    - tmp/cosim-arch-checker/mon/

## 需求

1. cpu 模式下主要用于验证cpu，现在只有retire的对比，需要借鉴 tmp/cosim-arch-checker, 使用更深层次的对比check
1. 移植 tmp/cosim-arch-checker/mon/ 到 src/mon
1. 先以 picorv32 为例子，集成mon到其中，注意打印信息到特定文件

## 测试 

1. picorv32: ./build.sh run cpu picorv32 hello
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

