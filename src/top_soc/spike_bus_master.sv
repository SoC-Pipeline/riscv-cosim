module spike_bus_master #(
	parameter [31:0] RESET_VECTOR = 32'h8000_0080
) (
	input clk,
	input resetn,
	output reg        mem_valid,
	output reg        mem_instr,
	input             mem_ready,
	output reg [31:0] mem_addr,
	output reg [31:0] mem_wdata,
	output reg [ 3:0] mem_wstrb,
	input      [31:0] mem_rdata,
	input      [31:0] irq
);
	import "DPI-C" function void spike_bus_init(input int reset_vector);
	import "DPI-C" function void spike_bus_step(
		input bit resetn,
		input bit mem_ready,
		input int unsigned mem_rdata,
		output bit out_mem_valid,
		output bit out_mem_instr,
		output int unsigned out_mem_addr,
		output int unsigned out_mem_wdata,
		output byte out_mem_wstrb
	);

	reg initialized = 1'b0;
	bit out_mem_valid;
	bit out_mem_instr;
	int unsigned out_mem_addr;
	int unsigned out_mem_wdata;
	byte out_mem_wstrb;
	wire mem_response = mem_valid && mem_ready;

	always @(posedge clk) begin
		if (!initialized) begin
			spike_bus_init(RESET_VECTOR);
			initialized <= 1'b1;
		end

		if (!resetn) begin
			mem_valid <= 1'b0;
			mem_instr <= 1'b0;
			mem_addr  <= 32'h0;
			mem_wdata <= 32'h0;
			mem_wstrb <= 4'h0;
		end else begin
			spike_bus_step(
				resetn,
				mem_response,
				mem_rdata,
				out_mem_valid,
				out_mem_instr,
				out_mem_addr,
				out_mem_wdata,
				out_mem_wstrb
			);

			mem_valid <= out_mem_valid;
			mem_instr <= out_mem_instr;
			mem_addr  <= out_mem_addr;
			mem_wdata <= out_mem_wdata;
			mem_wstrb <= out_mem_wstrb[3:0];
		end
	end
endmodule
