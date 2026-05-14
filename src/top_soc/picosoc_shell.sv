`timescale 1 ns / 1 ps

module picosoc_shell #(
	parameter integer MEM_WORDS = 256,
	parameter [31:0] RAM_BASE = 32'h8000_0000
) (
	input clk,
	input resetn,

	input        mem_valid,
	input        mem_instr,
	output       mem_ready,
	input  [31:0] mem_addr,
	input  [31:0] mem_wdata,
	input  [ 3:0] mem_wstrb,
	output [31:0] mem_rdata,

	output        iomem_valid,
	input         iomem_ready,
	output [ 3:0] iomem_wstrb,
	output [31:0] iomem_addr,
	output [31:0] iomem_wdata,
	input  [31:0] iomem_rdata
);
	wire [31:0] ram_rdata;
	wire [31:0] ram_size_bytes = 4 * MEM_WORDS;
	wire [31:0] ram_byte_addr = mem_addr - RAM_BASE;
	wire ram_sel = mem_valid &&
		(mem_addr >= RAM_BASE) &&
		(mem_addr < (RAM_BASE + ram_size_bytes));
	wire [21:0] ram_word_addr = ram_byte_addr[23:2];

	assign iomem_valid = mem_valid && (mem_addr[31:24] > 8'h01);
	assign iomem_wstrb = mem_wstrb;
	assign iomem_addr = mem_addr;
	assign iomem_wdata = mem_wdata;

	assign mem_ready = (iomem_valid && iomem_ready) || ram_sel;
	assign mem_rdata = ram_sel ? ram_rdata :
		((iomem_valid && iomem_ready) ? iomem_rdata : 32'h0000_0000);

	picosoc_shell_mem #(
		.WORDS(MEM_WORDS)
	) u_mem (
		.clk(clk),
		.wen(ram_sel ? mem_wstrb : 4'b0000),
		.addr(ram_word_addr),
		.wdata(mem_wdata),
		.rdata(ram_rdata)
	);

endmodule

module picosoc_shell_mem #(
	parameter integer WORDS = 256
) (
	input clk,
	input [3:0] wen,
	input [21:0] addr,
	input [31:0] wdata,
	output reg [31:0] rdata
);
	reg [31:0] mem [0:WORDS-1];
	localparam integer ADDR_BITS = $clog2(WORDS);
	wire [ADDR_BITS-1:0] mem_addr = addr[ADDR_BITS-1:0];
	assign rdata = mem[mem_addr];

	always @(posedge clk) begin
		if (wen[0]) mem[mem_addr][7:0] <= wdata[7:0];
		if (wen[1]) mem[mem_addr][15:8] <= wdata[15:8];
		if (wen[2]) mem[mem_addr][23:16] <= wdata[23:16];
		if (wen[3]) mem[mem_addr][31:24] <= wdata[31:24];
	end
endmodule
