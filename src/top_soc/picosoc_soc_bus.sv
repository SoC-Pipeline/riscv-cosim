`timescale 1 ns / 1 ps

module picosoc_soc_bus #(
	parameter integer MEM_WORDS = 32768
) (
	input clk,
	input resetn,
	input mem_valid,
	input mem_instr,
	output mem_ready,
	input [31:0] mem_addr,
	input [31:0] mem_wdata,
	input [3:0] mem_wstrb,
	output [31:0] mem_rdata,
	output logic finish_seen
);
	wire iomem_valid;
	reg iomem_ready;
	wire [3:0] iomem_wstrb;
	wire [31:0] iomem_addr;
	wire [31:0] iomem_wdata;
	reg [31:0] iomem_rdata;

	always @* begin
		iomem_ready = iomem_valid;
		iomem_rdata = 32'h0;
	end

	always @(posedge clk) begin
		if (!resetn)
			finish_seen <= 1'b0;
		else if (iomem_valid && iomem_ready && iomem_wstrb != 4'h0 &&
		         iomem_addr == 32'h2000_0000 && iomem_wdata == 32'd123456789)
			finish_seen <= 1'b1;
	end

	picosoc_soc #(
		.MEM_WORDS(MEM_WORDS)
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
endmodule
