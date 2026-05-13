# TODO

1. 深入理解代码: external/Cores-VeeR-EL2/
1. 理解top： external/Cores-VeeR-EL2/design/el2_veer_wrapper.sv
1. 理解testbench: external/Cores-VeeR-EL2/testbench/veer_wrapper.sv
1. 简单测试: cd external/Cores-VeeR-EL2/ && ./build.sh sim

## 需求

1. 集成 external/Cores-VeeR-EL2/ 到 src/top/tb_veer_el2.sv
1. 集成spike的cosim环境, 使 tb_veer_el2.sv 支持 cosim，注意参考已有的接口
1. 支持 firmware/ 下的testcase，tb 中注意reset_vector 配置

## 测试 

1. picorv32: ./build.sh run picorv32
1. ibex: ./build.sh clean && ./build.sh run ibex
1. veer_el2: ./build.sh run veer_el2

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

