module tb_veer_el2_monitor;
  import "DPI-C" function void veer_cosim_init(input string elf_path);
  import "DPI-C" function void veer_cosim_retire(input int unsigned pc, input int unsigned instr);
  import "DPI-C" function void veer_cosim_finish();
  import "DPI-C" function int unsigned veer_cosim_fail_count();

  localparam logic [31:0] SIM_PRINT_ADDR = 32'h1000_0000;
  localparam logic [31:0] SIM_FINISH_ADDR = 32'h2000_0000;
  localparam logic [31:0] SIM_FINISH_VALUE = 32'd123456789;

  string elf_path;
  bit cosim_ready;
  bit cosim_finished;
  bit has_last_retire;
  logic [31:0] last_retire_pc;
  logic [31:0] last_retire_instr;
  time last_retire_time;
  wire dut_trace_known = (^tb_top.trace_rv_i_address_ip !== 1'bx) &&
                         (^tb_top.trace_rv_i_insn_ip !== 1'bx);
  wire print_write = tb_top.rst_l &&
                     tb_top.lmem.awvalid &&
                     (tb_top.lmem.awaddr == SIM_PRINT_ADDR);
  wire finish_write = tb_top.rst_l &&
                      tb_top.lmem.awvalid &&
                      (tb_top.lmem.awaddr == SIM_FINISH_ADDR) &&
                      (tb_top.lmem.wdata[31:0] == SIM_FINISH_VALUE);

  function automatic bit is_print_char(logic [7:0] value);
    return value == 8'h09 ||
           value == 8'h0a ||
           value == 8'h0d ||
           (value >= 8'h20 && value < 8'h7f);
  endfunction

  initial begin
    if (!$value$plusargs("ELF_PATH=%s", elf_path)) begin
      elf_path = "build/firmware/hello/obj/firmware.elf";
    end

    @(posedge tb_top.rst_l);
    $display("=========================== VeeR EL2 RUN ===========================");
    $display("VeeR EL2 reset is released at time %.1f[ns]", $time);
    veer_cosim_init(elf_path);
    cosim_ready = 1'b1;
  end

  task automatic report_retire(input logic [31:0] pc, input logic [31:0] instr);
    if (!has_last_retire || last_retire_time != $time ||
        last_retire_pc != pc || last_retire_instr != instr) begin
      veer_cosim_retire(pc, instr);
      last_retire_pc = pc;
      last_retire_instr = instr;
      last_retire_time = $time;
      has_last_retire = 1'b1;
    end
  endtask

  task automatic finish_cosim;
    if (!cosim_finished) begin
      veer_cosim_finish();
      cosim_finished = 1'b1;
    end
  endtask

  always @(posedge tb_top.core_clk) begin
    if (cosim_ready && !cosim_finished &&
        tb_top.trace_rv_i_valid_ip &&
        dut_trace_known) begin
      report_retire(tb_top.trace_rv_i_address_ip, tb_top.trace_rv_i_insn_ip);
    end
  end

  always @(negedge tb_top.core_clk) begin
    if (finish_write) begin
      $display("Terminating simulation by shared firmware request: 0x%08x.", SIM_FINISH_VALUE);
      finish_cosim();
      if (veer_cosim_fail_count() != 0) begin
        $fatal;
      end
      $finish;
    end else if (print_write &&
                 (tb_top.lmem.wdata[63:32] == '0) &&
                 is_print_char(tb_top.lmem.wdata[7:0])) begin
      $write("%c", tb_top.lmem.wdata[7:0]);
    end
  end

  final begin
    finish_cosim();
  end
endmodule

bind tb_top tb_veer_el2_monitor u_tb_veer_el2_monitor();
