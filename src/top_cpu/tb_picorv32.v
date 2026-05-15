// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.

`timescale 1 ns / 1 ps


module tb_picorv32 #(
	parameter AXI_TEST = 0,
	parameter VERBOSE = 0,
	parameter [31:0] RESET_VECTOR = 32'h8000_0080
);

	integer i;
	reg clk = 1;
	reg resetn = 0;
	wire trap;

	always #5 clk = ~clk;


	initial begin
		repeat (100) @(posedge clk);
		resetn <= 1;
	end

	
`ifndef VERILATOR
	initial begin
		if ($test$plusargs("vcd")) begin
			$dumpfile("dump/tb_picorv32.vcd");
			$dumpvars(0, tb_picorv32);
		end
		repeat (1000000) @(posedge clk);
		$display("TIMEOUT");
		$finish;
	end
`else
	initial begin
		repeat (1000000) @(posedge clk);
		$display("TIMEOUT");
		$finish;
	end
`endif

	reg [1023:0] elf_path;
	reg [1023:0] pk_path;
	reg [1023:0] isa;

	initial begin
 		if (!$value$plusargs("ELF_PATH=%s", elf_path)) begin
      		$display("WARNING: ELF_PATH not set via plusargs. Using default value.");
			elf_path = "build/firmware/hello/obj/firmware.elf";
    	end	

    	if (!$value$plusargs("PK_PATH=%s", pk_path)) begin
      		$display("WARNING: PK_PATH not set via plusargs. Using default value.");
    	end

    	if (!$value$plusargs("ISA=%s", isa)) begin
      		$display("WARNING: ISA not set via plusargs. Using default value.");
		end

	end


	wire trace_valid;
	wire [35:0] trace_data;
	integer trace_file;

	initial begin
		if ($test$plusargs("trace")) begin
			trace_file = $fopen("dump/tb_picorv32.trace", "w");
			repeat (10) @(posedge clk);
			while (!trap) begin
				@(posedge clk);
				if (trace_valid)
					$fwrite(trace_file, "%x\n", trace_data);
			end
			$fclose(trace_file);
			$display("Finished writing tb_picorv32.trace.");
		end
	end

	picorv32_wrapper #(
		.AXI_TEST (AXI_TEST),
		.VERBOSE  (VERBOSE),
		.RESET_VECTOR (RESET_VECTOR)
	) top (
		.clk(clk),
		.resetn(resetn),
		.trap(trap),
		.trace_valid(trace_valid),
		.trace_data(trace_data)
	);
endmodule

module picorv32_wrapper #(
	parameter AXI_TEST = 0,
	parameter VERBOSE = 0,
	parameter [31:0] RESET_VECTOR = 32'h8000_0080
) (
	input clk,
	input resetn,
	output trap,
	output trace_valid,
	output [35:0] trace_data
);
	import "DPI-C" function int picorv32_cosim_init(input string elf_path);
	import "DPI-C" function int picorv32_cosim_monitor_retire(
		input int unsigned order,
		input int unsigned pc,
		input int unsigned instr,
		input int unsigned trap,
		input int unsigned rd_addr,
		input int unsigned rd_wdata,
		input int unsigned mem_addr,
		input int unsigned mem_rmask,
		input int unsigned mem_wmask,
		input int unsigned mem_rdata,
		input int unsigned mem_wdata,
		input int unsigned csr_addr,
		input int unsigned csr_rmask_lo,
		input int unsigned csr_rmask_hi,
		input int unsigned csr_rdata_lo,
		input int unsigned csr_rdata_hi,
		input int unsigned csr_wmask_lo,
		input int unsigned csr_wmask_hi,
		input int unsigned csr_wdata_lo,
		input int unsigned csr_wdata_hi
	);
	import "DPI-C" function void picorv32_cosim_finish();
	import "DPI-C" function int unsigned picorv32_cosim_fail_count();
	import "DPI-C" function void picorv32_cosim_set_test_result(input bit passed);

	wire tests_passed;
	reg [31:0] irq = 0;

	reg [15:0] count_cycle = 0;
	always @(posedge clk) count_cycle <= resetn ? count_cycle + 1 : 0;

	always @* begin
		irq = 0;
		irq[4] = &count_cycle[12:0];
		irq[5] = &count_cycle[15:0];
	end

	wire        mem_axi_awvalid;
	wire        mem_axi_awready;
	wire [31:0] mem_axi_awaddr;
	wire [ 2:0] mem_axi_awprot;

	wire        mem_axi_wvalid;
	wire        mem_axi_wready;
	wire [31:0] mem_axi_wdata;
	wire [ 3:0] mem_axi_wstrb;

	wire        mem_axi_bvalid;
	wire        mem_axi_bready;

	wire        mem_axi_arvalid;
	wire        mem_axi_arready;
	wire [31:0] mem_axi_araddr;
	wire [ 2:0] mem_axi_arprot;

	wire        mem_axi_rvalid;
	wire        mem_axi_rready;
	wire [31:0] mem_axi_rdata;

	axi4_memory #(
		.AXI_TEST (AXI_TEST),
		.VERBOSE  (VERBOSE)
	) mem (
		.clk             (clk             ),
		.mem_axi_awvalid (mem_axi_awvalid ),
		.mem_axi_awready (mem_axi_awready ),
		.mem_axi_awaddr  (mem_axi_awaddr  ),
		.mem_axi_awprot  (mem_axi_awprot  ),

		.mem_axi_wvalid  (mem_axi_wvalid  ),
		.mem_axi_wready  (mem_axi_wready  ),
		.mem_axi_wdata   (mem_axi_wdata   ),
		.mem_axi_wstrb   (mem_axi_wstrb   ),

		.mem_axi_bvalid  (mem_axi_bvalid  ),
		.mem_axi_bready  (mem_axi_bready  ),

		.mem_axi_arvalid (mem_axi_arvalid ),
		.mem_axi_arready (mem_axi_arready ),
		.mem_axi_araddr  (mem_axi_araddr  ),
		.mem_axi_arprot  (mem_axi_arprot  ),

		.mem_axi_rvalid  (mem_axi_rvalid  ),
		.mem_axi_rready  (mem_axi_rready  ),
		.mem_axi_rdata   (mem_axi_rdata   ),

		.tests_passed    (tests_passed    )
	);

	wire        rvfi_valid;
	wire [63:0] rvfi_order;
	wire [31:0] rvfi_insn;
	wire        rvfi_trap;
	wire        rvfi_halt;
	wire        rvfi_intr;
	wire [4:0]  rvfi_rs1_addr;
	wire [4:0]  rvfi_rs2_addr;
	wire [31:0] rvfi_rs1_rdata;
	wire [31:0] rvfi_rs2_rdata;
	wire [4:0]  rvfi_rd_addr;
	wire [31:0] rvfi_rd_wdata;
	wire [31:0] rvfi_pc_rdata;
	wire [31:0] rvfi_pc_wdata;
	wire [31:0] rvfi_mem_addr;
	wire [3:0]  rvfi_mem_rmask;
	wire [3:0]  rvfi_mem_wmask;
	wire [31:0] rvfi_mem_rdata;
	wire [31:0] rvfi_mem_wdata;
	wire [63:0] rvfi_csr_mcycle_rmask = uut.picorv32_core.rvfi_csr_mcycle_rmask;
	wire [63:0] rvfi_csr_mcycle_rdata = uut.picorv32_core.rvfi_csr_mcycle_rdata;
	wire [63:0] rvfi_csr_mcycle_wmask = uut.picorv32_core.rvfi_csr_mcycle_wmask;
	wire [63:0] rvfi_csr_mcycle_wdata = uut.picorv32_core.rvfi_csr_mcycle_wdata;
	wire [63:0] rvfi_csr_minstret_rmask = uut.picorv32_core.rvfi_csr_minstret_rmask;
	wire [63:0] rvfi_csr_minstret_rdata = uut.picorv32_core.rvfi_csr_minstret_rdata;
	wire [63:0] rvfi_csr_minstret_wmask = uut.picorv32_core.rvfi_csr_minstret_wmask;
	wire [63:0] rvfi_csr_minstret_wdata = uut.picorv32_core.rvfi_csr_minstret_wdata;
	wire        rvfi_has_mcycle_csr = |rvfi_csr_mcycle_rmask | |rvfi_csr_mcycle_wmask;
	wire        rvfi_has_minstret_csr = |rvfi_csr_minstret_rmask | |rvfi_csr_minstret_wmask;
	wire [11:0] rvfi_csr_addr = rvfi_has_mcycle_csr ? ((rvfi_csr_mcycle_rmask[63:32] != 0) ? 12'hC80 : 12'hC00) :
	                            rvfi_has_minstret_csr ? ((rvfi_csr_minstret_rmask[63:32] != 0) ? 12'hC82 : 12'hC02) :
	                            12'h000;
	wire [63:0] rvfi_csr_rmask = rvfi_has_mcycle_csr ? rvfi_csr_mcycle_rmask :
	                             rvfi_has_minstret_csr ? rvfi_csr_minstret_rmask : 64'h0;
	wire [63:0] rvfi_csr_rdata = rvfi_has_mcycle_csr ? rvfi_csr_mcycle_rdata :
	                             rvfi_has_minstret_csr ? rvfi_csr_minstret_rdata : 64'h0;
	wire [63:0] rvfi_csr_wmask = rvfi_has_mcycle_csr ? rvfi_csr_mcycle_wmask :
	                             rvfi_has_minstret_csr ? rvfi_csr_minstret_wmask : 64'h0;
	wire [63:0] rvfi_csr_wdata = rvfi_has_mcycle_csr ? rvfi_csr_mcycle_wdata :
	                             rvfi_has_minstret_csr ? rvfi_csr_minstret_wdata : 64'h0;

	picorv32_axi #(
`ifndef SYNTH_TEST
`ifdef SP_TEST
		.ENABLE_REGS_DUALPORT(0),
`endif
`ifdef COMPRESSED_ISA
		.COMPRESSED_ISA(1),
`endif
		.ENABLE_MUL(1),
		.ENABLE_DIV(1),
		.ENABLE_IRQ(1),
		.ENABLE_TRACE(1),
		.REGS_INIT_ZERO(1),
		.PROGADDR_RESET(RESET_VECTOR),
		.PROGADDR_IRQ(32'h8000_0010),
		.STACKADDR(32'h8001_8000)
`endif
	) uut (
		.clk            (clk            ),
		.resetn         (resetn         ),
		.trap           (trap           ),
		.mem_axi_awvalid(mem_axi_awvalid),
		.mem_axi_awready(mem_axi_awready),
		.mem_axi_awaddr (mem_axi_awaddr ),
		.mem_axi_awprot (mem_axi_awprot ),
		.mem_axi_wvalid (mem_axi_wvalid ),
		.mem_axi_wready (mem_axi_wready ),
		.mem_axi_wdata  (mem_axi_wdata  ),
		.mem_axi_wstrb  (mem_axi_wstrb  ),
		.mem_axi_bvalid (mem_axi_bvalid ),
		.mem_axi_bready (mem_axi_bready ),
		.mem_axi_arvalid(mem_axi_arvalid),
		.mem_axi_arready(mem_axi_arready),
		.mem_axi_araddr (mem_axi_araddr ),
		.mem_axi_arprot (mem_axi_arprot ),
		.mem_axi_rvalid (mem_axi_rvalid ),
		.mem_axi_rready (mem_axi_rready ),
		.mem_axi_rdata  (mem_axi_rdata  ),
		.irq            (irq            ),
		.rvfi_valid     (rvfi_valid     ),
		.rvfi_order     (rvfi_order     ),
		.rvfi_insn      (rvfi_insn      ),
		.rvfi_trap      (rvfi_trap      ),
		.rvfi_halt      (rvfi_halt      ),
		.rvfi_intr      (rvfi_intr      ),
		.rvfi_rs1_addr  (rvfi_rs1_addr  ),
		.rvfi_rs2_addr  (rvfi_rs2_addr  ),
		.rvfi_rs1_rdata (rvfi_rs1_rdata ),
		.rvfi_rs2_rdata (rvfi_rs2_rdata ),
		.rvfi_rd_addr   (rvfi_rd_addr   ),
		.rvfi_rd_wdata  (rvfi_rd_wdata  ),
		.rvfi_pc_rdata  (rvfi_pc_rdata  ),
		.rvfi_pc_wdata  (rvfi_pc_wdata  ),
		.rvfi_mem_addr  (rvfi_mem_addr  ),
		.rvfi_mem_rmask (rvfi_mem_rmask ),
		.rvfi_mem_wmask (rvfi_mem_wmask ),
		.rvfi_mem_rdata (rvfi_mem_rdata ),
		.rvfi_mem_wdata (rvfi_mem_wdata ),
		.trace_valid    (trace_valid    ),
		.trace_data     (trace_data     )
	);

	reg [1023:0] firmware_file;

	initial begin
		if (!$value$plusargs("firmware=%s", firmware_file)) begin
			firmware_file = "build/firmware/hello/obj/firmware.hex";

			$display("[DBG] picorv32 firmware_file = %0s", firmware_file);
		end
		
		$readmemh(firmware_file, mem.memory);
		$display("\n");

	end


	integer cycle_counter;
	always @(posedge clk) begin
		cycle_counter <= resetn ? cycle_counter + 1 : 0;
		if (resetn && trap) begin
`ifndef VERILATOR
			repeat (10) @(posedge clk);
`endif
			$display("TRAP after %1d clock cycles", cycle_counter);
			if (tests_passed) begin
				$display("ALL TESTS PASSED.");
			end
			else begin
				$display("ERROR!");
				if ($test$plusargs("noerror"))
					$finish;
			end
		end
	end

	reg cosim_ready = 0;
	reg [1023:0] cosim_elf_path;
	reg has_last_retire = 0;
	reg cosim_finished = 0;
	reg [63:0] last_retire_order;
	time last_retire_time;
	wire dut_trace_known = (^rvfi_pc_rdata !== 1'bx) &&
	                       (^rvfi_insn !== 1'bx) &&
	                       (^rvfi_rd_addr !== 1'bx) &&
	                       (^rvfi_rd_wdata !== 1'bx);

	initial begin
		int init_rc;
		@(posedge resetn);
		$display("=========================== COSIM RUN ===========================");
		$display("Reset is released at time %.1f[ns]", $time);

		if (!$value$plusargs("ELF_PATH=%s", cosim_elf_path)) begin
				cosim_elf_path = "build/firmware/hello/obj/firmware.elf";
		end
		init_rc = picorv32_cosim_init(cosim_elf_path);
		if (init_rc != 0) begin
			$display("FAIL: PicoRV32 cosim init failed.");
			$fatal;
		end
		cosim_ready = 1'b1;
	end

	task report_dut_retire;
		int retire_rc;
		begin
			if (!has_last_retire || last_retire_time != $time || last_retire_order != rvfi_order) begin
				retire_rc = picorv32_cosim_monitor_retire(
						rvfi_order[31:0], rvfi_pc_rdata, rvfi_insn, {31'b0, rvfi_trap},
						{27'b0, rvfi_rd_addr}, rvfi_rd_wdata, rvfi_mem_addr, {28'b0, rvfi_mem_rmask},
						{28'b0, rvfi_mem_wmask}, rvfi_mem_rdata, rvfi_mem_wdata,
						{20'b0, rvfi_csr_addr},
						rvfi_csr_rmask[31:0], rvfi_csr_rmask[63:32],
						rvfi_csr_rdata[31:0], rvfi_csr_rdata[63:32],
						rvfi_csr_wmask[31:0], rvfi_csr_wmask[63:32],
						rvfi_csr_wdata[31:0], rvfi_csr_wdata[63:32]);
				if (retire_rc != 0) begin
					$display("FAIL: PicoRV32 cosim retire mismatch.");
					$fatal;
				end
				last_retire_order = rvfi_order;
				last_retire_time = $time;
				has_last_retire = 1'b1;
			end
		end
	endtask

	task finish_cosim;
		begin
			if (!cosim_finished) begin
				if (cosim_ready && rvfi_valid && dut_trace_known) begin
					report_dut_retire();
				end
				picorv32_cosim_set_test_result(tests_passed);
				picorv32_cosim_finish();
				cosim_finished = 1'b1;
			end
		end
	endtask

	task maybe_report_dut_retire;
		begin
			if (!cosim_finished && cosim_ready && rvfi_valid && dut_trace_known) begin
				report_dut_retire();
			end
		end
	endtask

	always @(posedge clk) begin
		#1;
		maybe_report_dut_retire();
	end

	always @(posedge clk) begin
		if (resetn && (trap || tests_passed)) begin
			#2;
			finish_cosim();
			if (picorv32_cosim_fail_count() != 0) begin
				$fatal;
			end
			$finish;
		end
	end

	final begin
		finish_cosim();
	end


endmodule


module axi4_memory #(
	parameter AXI_TEST = 0,
	parameter VERBOSE = 0,
	parameter [31:0] MEM_BASE = 32'h8000_0000,
	parameter integer MEM_SIZE = 128*1024
) (
	/* verilator lint_off MULTIDRIVEN */

	input             clk,
	input             mem_axi_awvalid,
	output reg        mem_axi_awready,
	input      [31:0] mem_axi_awaddr,
	input      [ 2:0] mem_axi_awprot,

	input             mem_axi_wvalid,
	output reg        mem_axi_wready,
	input      [31:0] mem_axi_wdata,
	input      [ 3:0] mem_axi_wstrb,

	output reg        mem_axi_bvalid,
	input             mem_axi_bready,

	input             mem_axi_arvalid,
	output reg        mem_axi_arready,
	input      [31:0] mem_axi_araddr,
	input      [ 2:0] mem_axi_arprot,

	output reg        mem_axi_rvalid,
	input             mem_axi_rready,
	output reg [31:0] mem_axi_rdata,

	output reg        tests_passed
);
	localparam [31:0] SIM_MMIO_ADDR = 32'h1000_0000;
	localparam [7:0] RISCV_QUIT = 8'h00;
	localparam [7:0] RISCV_START = 8'h80;
	localparam [7:0] RISCV_FINISH = 8'h81;
	localparam [7:0] RISCV_FAIL = 8'h82;
	localparam [7:0] RISCV_PASS = 8'h83;

	reg [31:0]   memory [0:MEM_SIZE/4-1] /* verilator public */;

	reg verbose;
	initial verbose = $test$plusargs("verbose") || VERBOSE;

	reg axi_test;
	initial axi_test = $test$plusargs("axi_test") || AXI_TEST;

	initial begin
		mem_axi_awready = 0;
		mem_axi_wready = 0;
		mem_axi_bvalid = 0;
		mem_axi_arready = 0;
		mem_axi_rvalid = 0;
		tests_passed = 0;
	end

	reg [63:0] xorshift64_state = 64'd88172645463325252;

	task xorshift64_next;
		begin
			// see page 4 of Marsaglia, George (July 2003). "Xorshift RNGs". Journal of Statistical Software 8 (14).
			xorshift64_state = xorshift64_state ^ (xorshift64_state << 13);
			xorshift64_state = xorshift64_state ^ (xorshift64_state >>  7);
			xorshift64_state = xorshift64_state ^ (xorshift64_state << 17);
		end
	endtask

	reg [2:0] fast_axi_transaction = ~0;
	reg [4:0] async_axi_transaction = ~0;
	reg [4:0] delay_axi_transaction = 0;

	always @(posedge clk) begin
		if (axi_test) begin
				xorshift64_next;
				{fast_axi_transaction, async_axi_transaction, delay_axi_transaction} <= xorshift64_state;
		end
	end

	reg latched_raddr_en = 0;
	reg latched_waddr_en = 0;
	reg latched_wdata_en = 0;

	reg fast_raddr = 0;
	reg fast_waddr = 0;
	reg fast_wdata = 0;

	reg [31:0] latched_raddr;
	reg [31:0] latched_waddr;
	reg [31:0] latched_rmem_addr;
	reg [31:0] latched_wmem_addr;
	reg [31:0] latched_wdata;
	reg [ 3:0] latched_wstrb;
	reg        latched_rinsn;

	task handle_axi_arvalid; begin
		mem_axi_arready <= 1;
		latched_raddr = mem_axi_araddr;
		latched_rinsn = mem_axi_arprot[2];
		latched_raddr_en = 1;
		fast_raddr <= 1;
	end endtask

	task handle_axi_awvalid; begin
		mem_axi_awready <= 1;
		latched_waddr = mem_axi_awaddr;
		latched_waddr_en = 1;
		fast_waddr <= 1;
	end endtask

	task handle_axi_wvalid; begin
		mem_axi_wready <= 1;
		latched_wdata = mem_axi_wdata;
		latched_wstrb = mem_axi_wstrb;
		latched_wdata_en = 1;
		fast_wdata <= 1;
	end endtask

	task handle_axi_rvalid; begin
		latched_rmem_addr = latched_raddr - MEM_BASE;
		if (verbose)
			$display("RD: ADDR=%08x DATA=%08x%s", latched_raddr, memory[latched_rmem_addr >> 2], latched_rinsn ? " INSN" : "");

		if (latched_raddr >= MEM_BASE && latched_rmem_addr < MEM_SIZE) begin
			mem_axi_rdata <= memory[latched_rmem_addr >> 2];
			mem_axi_rvalid <= 1;
			latched_raddr_en = 0;
		end
		else begin
			$display("OUT-OF-BOUNDS MEMORY READ FROM %08x", latched_raddr);
			$finish;
		end
	end endtask

	task handle_axi_bvalid; begin
		latched_wmem_addr = latched_waddr - MEM_BASE;
		if (verbose)
			$display("WR: ADDR=%08x DATA=%08x STRB=%04b", latched_waddr, latched_wdata, latched_wstrb);

		if (latched_waddr >= MEM_BASE && latched_wmem_addr < MEM_SIZE) begin
			if (latched_wstrb[0]) memory[latched_wmem_addr >> 2][ 7: 0] <= latched_wdata[ 7: 0];
			if (latched_wstrb[1]) memory[latched_wmem_addr >> 2][15: 8] <= latched_wdata[15: 8];
			if (latched_wstrb[2]) memory[latched_wmem_addr >> 2][23:16] <= latched_wdata[23:16];
			if (latched_wstrb[3]) memory[latched_wmem_addr >> 2][31:24] <= latched_wdata[31:24];
		end
		else if (latched_waddr == SIM_MMIO_ADDR) begin
			case (latched_wdata[7:0])
			RISCV_START:
				$display("Firmware requested test start.");
			RISCV_PASS: begin
				tests_passed = 1;
				$display("PASS");
			end
			RISCV_FAIL: begin
				$display("FAIL");
				$finish;
			end
			RISCV_FINISH: begin
				tests_passed = 1;
				$display("Firmware requested test finish.");
			end
			RISCV_QUIT:
				;
			default: begin
				if (verbose) begin
					if (32 <= latched_wdata && latched_wdata < 128)
						$display("[%.1f ns] OUT: '%c'  (ADDR=%08x)", $time, latched_wdata[7:0], latched_waddr);
					else
						$display("[%.1f ns]   OUT: %3d    (ADDR=%08x)", $time, latched_wdata, latched_waddr);
				end
				else begin
					$write("%c", latched_wdata[7:0]);
`ifndef VERILATOR
					$fflush();
`endif
				end
			end
			endcase
		end
		else if (latched_waddr == 32'h2000_0000) begin
			if (latched_wdata == 123456789) begin
				tests_passed = 1;
				$display("[%.1f ns] OUT: %3d    (ADDR=%08x)", $time, latched_wdata, latched_waddr);
			end
		end
		else begin
			$display("OUT-OF-BOUNDS MEMORY WRITE TO %08x", latched_waddr);
			$finish;
		end
		mem_axi_bvalid <= 1;
		latched_waddr_en = 0;
		latched_wdata_en = 0;
	end endtask

	always @(negedge clk) begin
		if (mem_axi_arvalid && !(latched_raddr_en || fast_raddr) && async_axi_transaction[0]) handle_axi_arvalid;
		if (mem_axi_awvalid && !(latched_waddr_en || fast_waddr) && async_axi_transaction[1]) handle_axi_awvalid;
		if (mem_axi_wvalid  && !(latched_wdata_en || fast_wdata) && async_axi_transaction[2]) handle_axi_wvalid;
		if (!mem_axi_rvalid && latched_raddr_en && async_axi_transaction[3]) handle_axi_rvalid;
		if (!mem_axi_bvalid && latched_waddr_en && latched_wdata_en && async_axi_transaction[4]) handle_axi_bvalid;
	end

	always @(posedge clk) begin
		mem_axi_arready <= 0;
		mem_axi_awready <= 0;
		mem_axi_wready <= 0;

		fast_raddr <= 0;
		fast_waddr <= 0;
		fast_wdata <= 0;

		if (mem_axi_rvalid && mem_axi_rready) begin
			mem_axi_rvalid <= 0;
		end

		if (mem_axi_bvalid && mem_axi_bready) begin
			mem_axi_bvalid <= 0;
		end

		if (mem_axi_arvalid && mem_axi_arready && !fast_raddr) begin
			latched_raddr = mem_axi_araddr;
			latched_rinsn = mem_axi_arprot[2];
			latched_raddr_en = 1;
		end

		if (mem_axi_awvalid && mem_axi_awready && !fast_waddr) begin
			latched_waddr = mem_axi_awaddr;
			latched_waddr_en = 1;
		end

		if (mem_axi_wvalid && mem_axi_wready && !fast_wdata) begin
			latched_wdata = mem_axi_wdata;
			latched_wstrb = mem_axi_wstrb;
			latched_wdata_en = 1;
		end

		if (mem_axi_arvalid && !(latched_raddr_en || fast_raddr) && !delay_axi_transaction[0]) handle_axi_arvalid;
		if (mem_axi_awvalid && !(latched_waddr_en || fast_waddr) && !delay_axi_transaction[1]) handle_axi_awvalid;
		if (mem_axi_wvalid  && !(latched_wdata_en || fast_wdata) && !delay_axi_transaction[2]) handle_axi_wvalid;

		if (!mem_axi_rvalid && latched_raddr_en && !delay_axi_transaction[3]) handle_axi_rvalid;
		if (!mem_axi_bvalid && latched_waddr_en && latched_wdata_en && !delay_axi_transaction[4]) handle_axi_bvalid;
	end
endmodule
