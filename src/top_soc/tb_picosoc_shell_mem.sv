`timescale 1 ns / 1 ps

module tb_picosoc_shell_mem;
	localparam [31:0] RAM_BASE = 32'h8000_0000;
	reg clk = 1'b0;
	reg resetn = 1'b0;

	reg mem_valid = 1'b0;
	reg mem_instr = 1'b0;
	wire mem_ready;
	reg [31:0] mem_addr = 32'h0;
	reg [31:0] mem_wdata = 32'h0;
	reg [3:0] mem_wstrb = 4'h0;
	wire [31:0] mem_rdata;

	wire iomem_valid;
	reg iomem_ready = 1'b0;
	wire [3:0] iomem_wstrb;
	wire [31:0] iomem_addr;
	wire [31:0] iomem_wdata;
	reg [31:0] iomem_rdata = 32'h0;

	always #5 clk = ~clk;

	picosoc_shell #(
		.MEM_WORDS(256)
	) dut (
		.clk(clk),
		.resetn(resetn),
		.mem_valid(mem_valid),
		.mem_instr(mem_instr),
		.mem_ready(mem_ready),
		.mem_addr(mem_addr),
		.mem_wdata(mem_wdata),
		.mem_wstrb(mem_wstrb),
		.mem_rdata(mem_rdata),
		.iomem_valid(iomem_valid),
		.iomem_ready(iomem_ready),
		.iomem_wstrb(iomem_wstrb),
		.iomem_addr(iomem_addr),
		.iomem_wdata(iomem_wdata),
		.iomem_rdata(iomem_rdata)
	);

	task automatic bus_write(input [31:0] addr, input [31:0] data, input [3:0] wstrb);
	begin
		@(negedge clk);
		mem_valid = 1'b1;
		mem_addr = addr;
		mem_wdata = data;
		mem_wstrb = wstrb;
		mem_instr = 1'b0;
		while (!mem_ready) @(posedge clk);
		@(negedge clk);
		mem_valid = 1'b0;
		mem_wstrb = 4'h0;
	end
	endtask

	task automatic bus_read(input [31:0] addr, output [31:0] data);
	begin
		@(negedge clk);
		mem_valid = 1'b1;
		mem_addr = addr;
		mem_wdata = 32'h0;
		mem_wstrb = 4'h0;
		mem_instr = 1'b0;
		while (!mem_ready) @(posedge clk);
		data = mem_rdata;
		@(negedge clk);
		mem_valid = 1'b0;
	end
	endtask

	reg [31:0] rd0;
	reg [31:0] rd1;

	initial begin
		repeat (20000) @(posedge clk);
		$display("FAIL: timeout");
		$finish(1);
	end

	initial begin
		repeat (5) @(posedge clk);
		resetn = 1'b1;

		bus_write(RAM_BASE + 32'h0000_0010, 32'h1122_3344, 4'hF);
		bus_read(RAM_BASE + 32'h0000_0010, rd0);
		if (rd0 !== 32'h1122_3344) begin
			$display("FAIL: word readback mismatch rd0=%08x", rd0);
			$finish(1);
		end

		bus_write(RAM_BASE + 32'h0000_0010, 32'hAA00_0000, 4'h8);
		bus_read(RAM_BASE + 32'h0000_0010, rd1);
		if (rd1 !== 32'hAA22_3344) begin
			$display("FAIL: byte-lane write mismatch rd1=%08x", rd1);
			$finish(1);
		end

		$display("PASS: picosoc_shell memory read/write");
		$finish(0);
	end
endmodule
