`timescale 1 ns / 1 ps

module tb_picosoc_soc;
	parameter integer MEM_WORDS = 32768;
	parameter [31:0] RESET_VECTOR = 32'h8000_0080;
	import "DPI-C" function byte unsigned spike_bus_failed();
	import "DPI-C" function void spike_bus_finish();
	import "DPI-C" function void spike_bus_set_exit_code(input int code);

	localparam [31:0] SIM_MMIO_ADDR = 32'h1000_0000;
	localparam [7:0] RISCV_QUIT = 8'h00;
	localparam [7:0] RISCV_START = 8'h80;
	localparam [7:0] RISCV_FINISH = 8'h81;
	localparam [7:0] RISCV_FAIL = 8'h82;
	localparam [7:0] RISCV_PASS = 8'h83;

	reg clk = 1'b0;
	reg resetn = 1'b0;
	wire ser_tx;
	reg ser_rx = 1'b1;

	wire flash_csb;
	wire flash_clk;
	wire flash_io0_oe, flash_io1_oe, flash_io2_oe, flash_io3_oe;
	wire flash_io0_do, flash_io1_do, flash_io2_do, flash_io3_do;
	reg flash_io0_di = 1'b0, flash_io1_di = 1'b0, flash_io2_di = 1'b0, flash_io3_di = 1'b0;

	wire iomem_valid;
	wire [3:0] iomem_wstrb;
	wire [31:0] iomem_addr;
	wire [31:0] iomem_wdata;
	reg iomem_ready = 1'b0;
	reg [31:0] iomem_rdata = 32'h0;

	always #5 clk = ~clk;

	picosoc #(
		.MEM_WORDS(MEM_WORDS),
		.RAM_BASE(32'h8000_0000),
		.PROGADDR_RESET(RESET_VECTOR)
	) uut (
		.clk(clk),
		.resetn(resetn),
		.iomem_valid(iomem_valid),
		.iomem_ready(iomem_ready),
		.iomem_wstrb(iomem_wstrb),
		.iomem_addr(iomem_addr),
		.iomem_wdata(iomem_wdata),
		.iomem_rdata(iomem_rdata),
		.irq_5(1'b0),
		.irq_6(1'b0),
		.irq_7(1'b0),
		.ser_tx(ser_tx),
		.ser_rx(ser_rx),
		.flash_csb(flash_csb),
		.flash_clk(flash_clk),
		.flash_io0_oe(flash_io0_oe),
		.flash_io1_oe(flash_io1_oe),
		.flash_io2_oe(flash_io2_oe),
		.flash_io3_oe(flash_io3_oe),
		.flash_io0_do(flash_io0_do),
		.flash_io1_do(flash_io1_do),
		.flash_io2_do(flash_io2_do),
		.flash_io3_do(flash_io3_do),
		.flash_io0_di(flash_io0_di),
		.flash_io1_di(flash_io1_di),
		.flash_io2_di(flash_io2_di),
		.flash_io3_di(flash_io3_di)
	);

	reg [1023:0] firmware_file;
	initial begin
		if (!$value$plusargs("firmware=%s", firmware_file)) begin
			firmware_file = "build/firmware/hello/obj/firmware.hex";
		end
		$display("[SOC] firmware=%0s", firmware_file);
		$readmemh(firmware_file, uut.memory.mem);
		$display("[SOC] reset_word ram=%08x", uut.memory.mem[(RESET_VECTOR - 32'h8000_0000) >> 2]);
	end

	always @* begin
		iomem_ready = iomem_valid;
		iomem_rdata = 32'h0;
	end

	reg prev_iomem_fire = 1'b0;
	wire iomem_fire = iomem_valid && iomem_ready && (iomem_wstrb != 4'h0);
	wire iomem_fire_new = iomem_fire && !prev_iomem_fire;
	integer ram_read_count = 0;
	integer ram_write_count = 0;
	reg sim_done = 1'b0;
	reg finish_pending = 1'b0;
	integer finish_pending_code = 0;
	integer finish_delay = -1;

	function automatic bit is_print_char(input [7:0] value);
		is_print_char = value == 8'h09 ||
		                value == 8'h0a ||
		                value == 8'h0d ||
		                (value >= 8'h20 && value < 8'h7f);
	endfunction

	task automatic report_soc_counts;
		$display("[SOC] ram_reads=%0d ram_writes=%0d", ram_read_count, ram_write_count);
	endtask

	task automatic finish_soc(input int code);
		begin
			if (!sim_done) begin
				sim_done = 1'b1;
				report_soc_counts();
				spike_bus_set_exit_code(code);
				spike_bus_finish();
				$finish(code);
			end
		end
	endtask

	task automatic handle_sim_mmio(input [31:0] value);
		begin
			case (value[7:0])
			RISCV_START:
				$display("[SOC] firmware start");
			RISCV_PASS: begin
				$display("PASS");
				finish_pending = 1'b1;
				finish_pending_code = 0;
				finish_delay = -1;
			end
			RISCV_FAIL: begin
				$display("FAIL");
				finish_pending = 1'b1;
				finish_pending_code = 1;
				finish_delay = -1;
			end
			RISCV_FINISH: begin
				$display("[SOC] firmware finish");
				finish_pending = 1'b1;
				finish_pending_code = 0;
				finish_delay = -1;
			end
			RISCV_QUIT: begin
				$display("[SOC] firmware quit");
				if (finish_pending) begin
					finish_delay = 3;
				end
			end
			default:
				if (is_print_char(value[7:0])) begin
					$write("%c", value[7:0]);
				end else begin
					$display("[SOC] sim_mmio=0x%08x", value);
				end
			endcase
		end
	endtask

	always @(posedge clk) begin
		prev_iomem_fire <= iomem_fire;
		if (iomem_fire_new) begin
			if (iomem_addr == SIM_MMIO_ADDR) begin
				handle_sim_mmio(iomem_wdata);
			end
		end
		if (uut.mem_valid && uut.mem_ready && uut.ram_sel) begin
			if (uut.mem_wstrb == 4'h0) begin
				ram_read_count <= ram_read_count + 1;
			end else begin
				ram_write_count <= ram_write_count + 1;
			end
		end
		if (resetn && spike_bus_failed()) begin
			$display("FAIL: spike bus failed");
			finish_soc(1);
		end
		if (finish_pending && !sim_done && finish_delay >= 0) begin
			if (finish_delay > 0) begin
				finish_delay = finish_delay - 1;
			end else begin
				finish_soc(finish_pending_code);
			end
		end
	end

	initial begin
		repeat (8) @(posedge clk);
		resetn = 1'b1;
	end

	initial begin
		repeat (4000000) @(posedge clk);
		$display("FAIL: timeout");
		finish_soc(1);
	end

	final begin
		spike_bus_finish();
	end
endmodule
