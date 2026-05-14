#include "Vtb_picosoc_soc.h"
#include "verilated.h"

#include <cstdio>

extern "C" int spike_bus_exit_code();

int main(int argc, char** argv)
{
	std::setvbuf(stdout, nullptr, _IONBF, 0);
	std::setvbuf(stderr, nullptr, _IONBF, 0);
	Verilated::commandArgs(argc, argv);
	Vtb_picosoc_soc top;
	while (!Verilated::gotFinish()) {
		top.eval();
		Verilated::timeInc(1);
	}
	top.final();
	return spike_bus_exit_code();
}
