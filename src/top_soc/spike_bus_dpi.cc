#include "elf_utils.h"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <riscv/common.h>
#include <riscv/config.h>
#include <riscv/isa_parser.h>
#include <riscv/processor.h>
#include <riscv/simif.h>
#include <riscv/trap.h>

namespace {

constexpr uint64_t kMaxSteps = 2000000;

class SoCSimIf;

struct BusRequest {
	bool valid = false;
	bool instr = false;
	uint32_t addr = 0;
	uint32_t wdata = 0;
	uint8_t wstrb = 0;
	uint32_t rdata = 0;
	bool response_ready = false;
};

struct Context {
	std::mutex mutex;
	std::condition_variable cv;
	uint32_t reset_vector = 0x80000080u;
	BusRequest req;
	bool initialized = false;
	bool running = false;
	bool failed = false;
	bool stop = false;
	bool wait_ready_low = false;
	uint64_t steps = 0;

	std::unique_ptr<isa_parser_t> isa;
	std::unique_ptr<SoCSimIf> simif;
	std::unique_ptr<processor_t> cpu;
	std::unique_ptr<FILE, int (*)(FILE*)> log_file{nullptr, &std::fclose};
	std::thread worker;
} g_ctx;

std::atomic<int> g_exit_code{0};

uint32_t pack_store_data(reg_t addr, size_t len, const uint8_t* bytes)
{
	uint32_t value = 0;
	const uint32_t shift = static_cast<uint32_t>(addr & 0x3u) * 8u;
	for (size_t i = 0; i < len && i < 4; ++i) {
		value |= static_cast<uint32_t>(bytes[i]) << (shift + i * 8u);
	}
	return value;
}

uint8_t pack_store_strobe(reg_t addr, size_t len)
{
	uint8_t strobe = 0;
	const uint32_t lane = static_cast<uint32_t>(addr & 0x3u);
	for (size_t i = 0; i < len && (lane + i) < 4; ++i) {
		strobe |= static_cast<uint8_t>(1u << (lane + i));
	}
	return strobe;
}

void unpack_load_data(reg_t addr, size_t len, uint32_t rdata, uint8_t* bytes)
{
	const uint32_t shift = static_cast<uint32_t>(addr & 0x3u) * 8u;
	for (size_t i = 0; i < len; ++i) {
		bytes[i] = static_cast<uint8_t>((rdata >> (shift + i * 8u)) & 0xffu);
	}
}

bool issue_bus_request(reg_t addr, size_t len, uint8_t* load_bytes, const uint8_t* store_bytes, bool instr)
{
	if (len == 0 || len > 4 || ((addr & 0x3u) + len) > 4) {
		return false;
	}

	std::unique_lock<std::mutex> lock(g_ctx.mutex);
	g_ctx.cv.wait(lock, [] { return !g_ctx.req.valid || g_ctx.stop; });
	if (g_ctx.stop || g_ctx.failed) {
		return false;
	}

	g_ctx.req.valid = true;
	g_ctx.req.instr = instr;
	g_ctx.req.addr = static_cast<uint32_t>(addr) & ~0x3u;
	g_ctx.req.wdata = store_bytes ? pack_store_data(addr, len, store_bytes) : 0;
	g_ctx.req.wstrb = store_bytes ? pack_store_strobe(addr, len) : 0;
	g_ctx.req.rdata = 0;
	g_ctx.req.response_ready = false;
	g_ctx.cv.notify_all();

	g_ctx.cv.wait(lock, [] { return g_ctx.req.response_ready || g_ctx.stop || g_ctx.failed; });
	if (g_ctx.failed || (!g_ctx.req.response_ready && g_ctx.stop)) {
		return false;
	}

	if (!store_bytes) {
		unpack_load_data(addr, len, g_ctx.req.rdata, load_bytes);
	}

	g_ctx.req.valid = false;
	g_ctx.req.response_ready = false;
	g_ctx.cv.notify_all();
	return true;
}

class SoCSimIf final : public simif_t {
public:
	char* addr_to_mem(reg_t addr) override
	{
		(void)addr;
		return nullptr;
	}

	bool mmio_load(reg_t addr, size_t len, uint8_t* bytes) override
	{
		return issue_bus_request(addr, len, bytes, nullptr, true);
	}

	bool mmio_store(reg_t addr, size_t len, const uint8_t* bytes) override
	{
		return issue_bus_request(addr, len, nullptr, bytes, false);
	}

	void proc_reset(unsigned id) override { (void)id; }

	const char* get_symbol(uint64_t addr) override
	{
		(void)addr;
		return nullptr;
	}
};

void mark_failed()
{
	std::lock_guard<std::mutex> lock(g_ctx.mutex);
	g_ctx.failed = true;
	g_ctx.cv.notify_all();
}

void run_spike()
{
	try {
		while (true) {
			{
				std::lock_guard<std::mutex> lock(g_ctx.mutex);
				if (g_ctx.stop || g_ctx.failed) {
					return;
				}
				if (g_ctx.steps >= kMaxSteps) {
					g_ctx.failed = true;
					g_exit_code.store(1);
					g_ctx.cv.notify_all();
					return;
				}
			}

			g_ctx.cpu->step(1);

			{
				std::lock_guard<std::mutex> lock(g_ctx.mutex);
				g_ctx.steps++;
			}
		}
	} catch (trap_t&) {
		mark_failed();
	} catch (std::exception& e) {
		std::cerr << "[SOC-SPIKE] failed: " << e.what() << std::endl;
		mark_failed();
	} catch (...) {
		mark_failed();
	}
}

bool init_spike_once()
{
	if (g_ctx.initialized) {
		return true;
	}

	std::string elf_path = std::getenv("TEST_ELF") ? std::getenv("TEST_ELF") : "";
	std::string isa = std::getenv("MY_ISA") ? std::getenv("MY_ISA") : "RV32IMC";
	std::string log_path = std::getenv("SOC_SPIKE_COMMIT_LOG") ? std::getenv("SOC_SPIKE_COMMIT_LOG") : "";
	if (elf_path.empty()) {
		return false;
	}

	g_ctx.simif = std::make_unique<SoCSimIf>();
	if (!log_path.empty()) {
		FILE* file = std::fopen(log_path.c_str(), "w");
		if (!file) {
			std::cerr << "[SOC-SPIKE] failed to open commit log: " << log_path << std::endl;
			return false;
		}
		g_ctx.log_file.reset(file);
	}

	g_ctx.isa = std::make_unique<isa_parser_t>(isa.c_str(), DEFAULT_PRIV);
	g_ctx.cpu = std::make_unique<processor_t>(g_ctx.isa.get(), DEFAULT_VARCH, g_ctx.simif.get(), 0, false,
	                                          g_ctx.log_file.get(), std::cerr);
	g_ctx.cpu->reset();
	g_ctx.cpu->get_state()->pc = static_cast<reg_t>(read_elf_entry(elf_path));

	if (!log_path.empty()) {
		g_ctx.cpu->set_debug(true);
#ifdef RISCV_ENABLE_COMMITLOG
		g_ctx.cpu->enable_log_commits();
#endif
	}

	g_ctx.initialized = true;
	return true;
}

void start_spike_once()
{
	if (g_ctx.running) {
		return;
	}
	g_ctx.running = true;
	g_ctx.worker = std::thread(run_spike);
}

void clear_outputs(unsigned char* out_mem_valid, unsigned char* out_mem_instr, uint32_t* out_mem_addr,
                   uint32_t* out_mem_wdata, unsigned char* out_mem_wstrb)
{
	*out_mem_valid = 0;
	*out_mem_instr = 0;
	*out_mem_addr = 0;
	*out_mem_wdata = 0;
	*out_mem_wstrb = 0;
}

} // namespace

extern "C" {

void spike_bus_init(int reset_vector)
{
	std::lock_guard<std::mutex> lock(g_ctx.mutex);
	g_ctx.reset_vector = static_cast<uint32_t>(reset_vector);
}

void spike_bus_step(unsigned char resetn, unsigned char mem_ready, uint32_t mem_rdata,
                    unsigned char* out_mem_valid, unsigned char* out_mem_instr,
                    uint32_t* out_mem_addr, uint32_t* out_mem_wdata, unsigned char* out_mem_wstrb)
{
	clear_outputs(out_mem_valid, out_mem_instr, out_mem_addr, out_mem_wdata, out_mem_wstrb);

	if (!resetn) {
		return;
	}

	if (!init_spike_once()) {
		mark_failed();
		return;
	}
	start_spike_once();

	std::lock_guard<std::mutex> lock(g_ctx.mutex);
	if (g_ctx.wait_ready_low) {
		if (!mem_ready) {
			g_ctx.wait_ready_low = false;
		}
		return;
	}

	if (!g_ctx.req.valid) {
		return;
	}

	*out_mem_valid = 1;
	*out_mem_instr = g_ctx.req.instr ? 1 : 0;
	*out_mem_addr = g_ctx.req.addr;
	*out_mem_wdata = g_ctx.req.wdata;
	*out_mem_wstrb = g_ctx.req.wstrb;

	if (mem_ready && !g_ctx.req.response_ready) {
		g_ctx.req.rdata = mem_rdata;
		g_ctx.req.response_ready = true;
		g_ctx.wait_ready_low = true;
		g_ctx.cv.notify_all();
	}
}

unsigned char spike_bus_failed()
{
	std::lock_guard<std::mutex> lock(g_ctx.mutex);
	return g_ctx.failed ? 1 : 0;
}

void spike_bus_finish()
{
	{
		std::lock_guard<std::mutex> lock(g_ctx.mutex);
		g_ctx.stop = true;
		g_ctx.cv.notify_all();
	}
	if (g_ctx.worker.joinable()) {
		g_ctx.worker.join();
	}
}

void spike_bus_set_exit_code(int code)
{
	g_exit_code.store(code);
}

int spike_bus_exit_code()
{
	return g_exit_code.load();
}

} // extern "C"
