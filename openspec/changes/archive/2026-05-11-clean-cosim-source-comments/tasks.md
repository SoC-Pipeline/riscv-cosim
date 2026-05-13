## 1. Cosim Bridge Cleanup

- [x] 1.1 Remove obsolete commented-out VPI/DPI code blocks from `src/cosim/spike_dpi.cc`.
- [x] 1.2 Replace personal markers and non-English comments in active `src/cosim/spike_dpi.cc`, `src/cosim/sim.h`, and `src/cosim/htif.h`.
- [x] 1.3 Confirm `src/cosim/spike_dpi_thread.cc` remains present.

## 2. Testbench And Firmware Cleanup

- [x] 2.1 Remove obsolete commented-out debug code from `src/top/testbench.v`.
- [x] 2.2 Replace personal markers in `src/top/testbench.v` with functional names or comments.
- [x] 2.3 Clean small firmware support files while avoiding broad generated assembly rewrites.

## 3. Verification

- [x] 3.1 Search active paths for remaining personal markers, non-English comments, and obvious commented-out dead code.
- [x] 3.2 Run `make all` with the required modules loaded and confirm zero compare failures.
- [x] 3.3 Review final diff and OpenSpec status.
