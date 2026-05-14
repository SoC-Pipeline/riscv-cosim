## 1. Remove legacy unused cosim path

- [x] 1.1 Delete `src/cosim/spike_dpi_thread.cc`
- [x] 1.2 Delete `src/cosim/sim.h`
- [x] 1.3 Delete `src/cosim/htif.h`
- [x] 1.4 Run smoke regression (`picorv32/ibex/veer_el2` + `hello`)

## 2. Unify config construction

- [x] 2.1 Add shared `CosimConfig` builder keyed by `cpu_name`
- [x] 2.2 Switch `spike_dpi.cc` to shared builder
- [x] 2.3 Switch `tb_veer_el2_cosim.cc` to shared builder
- [x] 2.4 Switch `tb_ibex_cosim.cc` to shared builder
- [x] 2.5 Run smoke regression (`picorv32/ibex/veer_el2` + `hello`)

## 3. Unify log naming policy with compatibility

- [x] 3.1 Add default naming policy `dump/<cpu_name>_{cosim_result,spike_commit}.log`
- [x] 3.2 Add generic override env vars and keep existing per-target env compatibility
- [x] 3.3 Update `build.sh` help text
- [x] 3.4 Run full regression (`picorv32/ibex/veer_el2` + `hello,pico_test`)

## 4. Documentation alignment

- [x] 4.1 Update `docs/arch.md` for removed legacy path
- [x] 4.2 Document unified config/log policy and env precedence
- [x] 4.3 Validate OpenSpec change
