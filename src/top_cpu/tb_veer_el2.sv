module tb_veer_el2_monitor;
  import "DPI-C" function void veer_cosim_init(input string elf_path);
  import "DPI-C" function void veer_cosim_retire(input int unsigned pc,
                                                 input int unsigned instr,
                                                 input bit trap,
                                                 input bit gpr_valid,
                                                 input int unsigned rd_addr,
                                                 input int unsigned rd_wdata,
                                                 input bit csr_valid,
                                                 input int unsigned csr_addr,
                                                 input int unsigned csr_wdata);
  import "DPI-C" function void veer_cosim_finish();
  import "DPI-C" function int unsigned veer_cosim_fail_count();

  localparam logic [31:0] SIM_PRINT_ADDR = 32'h1000_0000;
  localparam logic [31:0] SIM_FINISH_ADDR = 32'h2000_0000;
  localparam logic [31:0] SIM_FINISH_VALUE = 32'd123456789;
  localparam logic [7:0] RISCV_QUIT = 8'h00;
  localparam logic [7:0] RISCV_START = 8'h80;
  localparam logic [7:0] RISCV_FINISH = 8'h81;
  localparam logic [7:0] RISCV_FAIL = 8'h82;
  localparam logic [7:0] RISCV_PASS = 8'h83;

  string elf_path;
  bit cosim_ready;
  bit cosim_finished;
  bit has_last_retire;
  bit sim_mmio_debug;
  logic [31:0] last_retire_pc;
  logic [31:0] last_retire_instr;
  time last_retire_time;
  logic [31:0] aw_queue[8];
  int unsigned aw_queue_head;
  int unsigned aw_queue_tail;
  int unsigned aw_queue_count;
  wire dut_trace_known = (^tb_top.trace_rv_i_address_ip !== 1'bx) &&
                         (^tb_top.trace_rv_i_insn_ip !== 1'bx);

  function automatic logic [31:0] write_lane_value(
      logic [31:0] addr,
      logic [63:0] data,
      logic [7:0] strb);
    logic [31:0] lane_value;

    lane_value = 32'b0;
    for (int i = 0; i < 4; i++) begin
      int lane;

      lane = addr[2:0] + i;
      if (lane < 8 && strb[lane]) begin
        lane_value[i * 8 +: 8] = data[lane * 8 +: 8];
      end
    end

    return lane_value;
  endfunction

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
    sim_mmio_debug = $test$plusargs("VEER_SIM_MMIO_DEBUG");

    @(posedge tb_top.rst_l);
    $display("=========================== VeeR EL2 RUN ===========================");
    $display("VeeR EL2 reset is released at time %.1f[ns]", $time);
    veer_cosim_init(elf_path);
    cosim_ready = 1'b1;
  end

  task automatic report_retire(input logic [31:0] pc, input logic [31:0] instr,
                               input logic trap, input logic gpr_valid,
                               input logic [4:0] rd_addr,
                               input logic [31:0] rd_wdata,
                               input logic csr_valid,
                               input logic [11:0] csr_addr,
                               input logic [31:0] csr_wdata);
    if (!has_last_retire || last_retire_time != $time ||
        last_retire_pc != pc || last_retire_instr != instr) begin
      veer_cosim_retire(pc, instr, trap, gpr_valid, {27'b0, rd_addr}, rd_wdata,
                        csr_valid, {20'b0, csr_addr}, csr_wdata);
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

  task automatic handle_sim_print(input logic [31:0] value);
    case (value[7:0])
      RISCV_START:
        $display("Firmware requested test start.");
      RISCV_PASS: begin
        $display("PASS");
        finish_cosim();
        if (veer_cosim_fail_count() != 0) begin
          $fatal;
        end
        $finish;
      end
      RISCV_FAIL: begin
        $display("FAIL");
        finish_cosim();
        $fatal;
      end
      RISCV_FINISH: begin
        $display("Firmware requested test finish.");
        finish_cosim();
        $finish;
      end
      RISCV_QUIT:
        ;
      default:
        if (is_print_char(value[7:0])) begin
          $write("%c", value[7:0]);
        end
    endcase
  endtask

  always @(posedge tb_top.core_clk) begin
    if (cosim_ready && !cosim_finished &&
        tb_top.trace_rv_i_valid_ip &&
        dut_trace_known) begin
      report_retire(tb_top.trace_rv_i_address_ip, tb_top.trace_rv_i_insn_ip,
                    tb_top.trace_rv_i_exception_ip |
                    tb_top.trace_rv_i_interrupt_ip,
                    tb_top.wb_valid && (tb_top.wb_dest != 5'd0),
                    tb_top.wb_dest,
                    tb_top.wb_data,
                    tb_top.wb_csr_valid,
                    tb_top.wb_csr_dest,
                    tb_top.wb_csr_data);
    end
  end

  always @(negedge tb_top.core_clk) begin
    logic [31:0] sim_addr;
    logic [31:0] sim_value;

    if (!tb_top.rst_l) begin
      aw_queue_head = 0;
      aw_queue_tail = 0;
      aw_queue_count = 0;
    end else begin
      if (tb_top.lmem.awvalid) begin
        if (aw_queue_count < 8) begin
          aw_queue[aw_queue_tail] = tb_top.lmem.awaddr;
          aw_queue_tail = (aw_queue_tail + 1) % 8;
          aw_queue_count = aw_queue_count + 1;
        end else begin
          $display("FAIL: VeeR write-address monitor queue overflow");
          finish_cosim();
          $fatal;
        end
      end

      if (tb_top.lmem.wvalid && aw_queue_count != 0) begin
        sim_addr = aw_queue[aw_queue_head];
        sim_value = write_lane_value(sim_addr, tb_top.lmem.wdata,
                                     tb_top.lmem.wstrb);
        aw_queue_head = (aw_queue_head + 1) % 8;
        aw_queue_count = aw_queue_count - 1;

        if (sim_mmio_debug &&
            (sim_addr == SIM_PRINT_ADDR || sim_addr == SIM_FINISH_ADDR)) begin
          $display("[VEER SIM MMIO] addr=0x%08x wdata=0x%016x wstrb=0x%02x value=0x%08x",
                   sim_addr, tb_top.lmem.wdata, tb_top.lmem.wstrb, sim_value);
        end

        if (sim_addr == SIM_FINISH_ADDR && sim_value == SIM_FINISH_VALUE) begin
          $display("Terminating simulation by shared firmware request: 0x%08x.",
                   SIM_FINISH_VALUE);
          finish_cosim();
          if (veer_cosim_fail_count() != 0) begin
            $fatal;
          end
          $finish;
        end else if (sim_addr == SIM_PRINT_ADDR) begin
          handle_sim_print(sim_value);
        end
      end
    end
  end

  final begin
    finish_cosim();
  end
endmodule

bind tb_top tb_veer_el2_monitor u_tb_veer_el2_monitor();
