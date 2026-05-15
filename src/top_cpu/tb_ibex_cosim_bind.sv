// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

module tb_ibex_cosim_bind;
  bind tb_ibex tb_ibex_monitor_bind u_tb_ibex_monitor_bind (
      .clk_i (IO_CLK),
      .rst_ni (IO_RST_N)
    );

  bind tb_ibex ibex_simple_system_cosim_checker #(
      .SecureIbex,
      .ICache,
      .PMPEnable,
      .PMPGranularity,
      .PMPNumRegions,
      .MHPMCounterNum,
      .DmBaseAddr (RamBaseAddr),
      .DmAddrMask (32'h0000_0003)
    ) u_tb_ibex_cosim_checker_bind (
      .clk_i            (IO_CLK),
      .rst_ni           (IO_RST_N),

      .host_dmem_req    (host_req[CoreD]),
      .host_dmem_gnt    (host_gnt[CoreD]),
      .host_dmem_we     (host_we[CoreD]),
      .host_dmem_addr   (host_addr[CoreD]),
      .host_dmem_be     (host_be[CoreD]),
      .host_dmem_wdata  (host_wdata[CoreD]),

      .host_dmem_rvalid (host_rvalid[CoreD]),
      .host_dmem_rdata  (host_rdata[CoreD]),
      .host_dmem_err    (host_err[CoreD])
    );
endmodule

/* verilator lint_off DECLFILENAME */
/* verilator lint_off BLKSEQ */
/* verilator lint_off SYNCASYNCNET */
module tb_ibex_monitor_bind (
  input logic clk_i,
  input logic rst_ni
);
  import "DPI-C" function void ibex_mon_init();
  import "DPI-C" function int ibex_mon_retire(input int unsigned order,
                                              input int unsigned pc,
                                              input int unsigned instr,
                                              input bit trap,
                                              input bit gpr_valid,
                                              input int unsigned rd_addr,
                                              input int unsigned rd_wdata);
  import "DPI-C" function void ibex_mon_finish();

  bit mon_ready;
  bit mon_finished;
  logic [63:0] last_retire_order;
  time last_retire_time;
  wire dut_retire_known = (^u_top.rvfi_pc_rdata !== 1'bx) &&
                          (^u_top.rvfi_insn !== 1'bx) &&
                          (^u_top.rvfi_rd_addr !== 1'bx) &&
                          (^u_top.rvfi_rd_wdata !== 1'bx);

  initial begin
    ibex_mon_init();
    mon_ready = 1'b1;
  end

  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      last_retire_order <= '0;
      last_retire_time <= 0;
    end else if (u_top.rvfi_valid) begin
      if (mon_ready && dut_retire_known &&
          (last_retire_time != $time || last_retire_order != u_top.rvfi_order)) begin
        void'(ibex_mon_retire(
            u_top.rvfi_order[31:0],
            u_top.rvfi_pc_rdata,
            u_top.rvfi_insn,
            u_top.rvfi_trap,
            u_top.rvfi_rd_addr != 5'd0,
            {27'b0, u_top.rvfi_rd_addr},
            u_top.rvfi_rd_wdata));
        last_retire_order <= u_top.rvfi_order;
        last_retire_time <= $time;
      end
    end
  end

  final begin
    if (!mon_finished) begin
      ibex_mon_finish();
      mon_finished = 1'b1;
    end
  end
endmodule
/* verilator lint_on SYNCASYNCNET */
/* verilator lint_on BLKSEQ */
/* verilator lint_on DECLFILENAME */
