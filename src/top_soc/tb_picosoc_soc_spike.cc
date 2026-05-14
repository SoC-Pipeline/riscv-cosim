#include <verilated.h>

#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "Vpicosoc_soc_bus.h"

#include "riscv/isa_parser.h"
#include "riscv/mmu.h"
#include "riscv/processor.h"
#include "riscv/simif.h"
#include "riscv/trap.h"

namespace {

uint64_t read_elf_entry(const std::string &path)
{
	FILE *f = std::fopen(path.c_str(), "rb");
	if (!f) {
		throw std::runtime_error("failed to open ELF: " + path);
	}
	uint8_t hdr[64];
	const size_t n = std::fread(hdr, 1, sizeof(hdr), f);
	std::fclose(f);
	if (n < 52 || hdr[0] != 0x7f || hdr[1] != 'E' || hdr[2] != 'L' || hdr[3] != 'F') {
		throw std::runtime_error("invalid ELF header: " + path);
	}
	if (hdr[4] == 1) {
		uint32_t e = hdr[24] | (hdr[25] << 8) | (hdr[26] << 16) | (hdr[27] << 24);
		return e;
	}
	if (hdr[4] == 2) {
		uint64_t e = 0;
		for (int i = 0; i < 8; ++i) {
			e |= (uint64_t)hdr[24 + i] << (8 * i);
		}
		return e;
	}
	throw std::runtime_error("unsupported ELF class");
}

std::vector<uint8_t> read_file(const std::string &path)
{
	FILE *f = std::fopen(path.c_str(), "rb");
	if (!f) throw std::runtime_error("failed to open file: " + path);
	std::fseek(f, 0, SEEK_END);
	long sz = std::ftell(f);
	if (sz <= 0) {
		std::fclose(f);
		throw std::runtime_error("invalid file size: " + path);
	}
	std::rewind(f);
	std::vector<uint8_t> buf(static_cast<size_t>(sz));
	size_t n = std::fread(buf.data(), 1, buf.size(), f);
	std::fclose(f);
	if (n != buf.size()) throw std::runtime_error("short read: " + path);
	return buf;
}

uint16_t rd16(const uint8_t *p) { return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }
uint32_t rd32(const uint8_t *p) { return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }
uint64_t rd64(const uint8_t *p) { return (uint64_t)rd32(p) | ((uint64_t)rd32(p + 4) << 32); }

class TbBusSimif : public simif_t {
public:
	explicit TbBusSimif(Vpicosoc_soc_bus *tb) : tb_(tb) {}

	struct BusStats {
		uint64_t rd_total = 0;
		uint64_t wr_total = 0;
		uint64_t rd_ram = 0;
		uint64_t wr_ram = 0;
	};

	char *addr_to_mem(reg_t) override { return nullptr; }

	bool mmio_load(reg_t addr, size_t len, uint8_t *bytes) override
	{
		if (len != 1 && len != 2 && len != 4) {
			return false;
		}
		const uint32_t a = static_cast<uint32_t>(addr);
		const uint32_t aligned = a & ~0x3u;
		const uint32_t lane = a & 0x3u;
		uint64_t data64 = static_cast<uint64_t>(bus_read(aligned));
		if (lane + len > 4) {
			data64 |= static_cast<uint64_t>(bus_read(aligned + 4)) << 32;
		}
		for (size_t i = 0; i < len; ++i) {
			bytes[i] = static_cast<uint8_t>((data64 >> (8 * (lane + i))) & 0xFF);
		}
		return true;
	}

    bool mmio_store(reg_t addr, size_t len, const uint8_t *bytes) override
    {
        if (len != 1 && len != 2 && len != 4) {
            return false;
        }
        if (addr == 0x10000000u && len >= 1) {
            const unsigned char ch = bytes[0];
            if (ch >= 32 && ch < 127) {
                std::cout << static_cast<char>(ch) << std::flush;
            }
        }
        const uint32_t a = static_cast<uint32_t>(addr);
        const uint32_t aligned = a & ~0x3u;
        const uint32_t lane = a & 0x3u;
        uint32_t data = 0;
        uint32_t wstrb = 0;
        for (size_t i = 0; i < len; ++i) {
            data |= (uint32_t)bytes[i] << (8 * (lane + i));
            wstrb |= 1u << (lane + i);
        }
        bus_write(aligned, data, wstrb);
        return true;
    }

	void proc_reset(unsigned) override {}
	const char *get_symbol(uint64_t) override { return nullptr; }

	void set_clock(int clk)
	{
		tb_->clk = clk;
		tb_->eval();
	}

	void tick()
	{
		set_clock(0);
		set_clock(1);
	}

	void reset()
	{
		tb_->mem_valid = 0;
		tb_->mem_instr = 0;
		tb_->mem_addr = 0;
		tb_->mem_wdata = 0;
		tb_->mem_wstrb = 0;
		tb_->resetn = 0;
		for (int i = 0; i < 8; ++i) tick();
		tb_->resetn = 1;
		for (int i = 0; i < 4; ++i) tick();
	}
	void preload_elf(const std::string &elf_path)
	{
		std::vector<uint8_t> elf = read_file(elf_path);
		const uint8_t *hdr = elf.data();
		if (elf.size() < 0x40 || hdr[0] != 0x7f || hdr[1] != 'E' || hdr[2] != 'L' || hdr[3] != 'F') {
			throw std::runtime_error("invalid ELF header: " + elf_path);
		}
		bool is64 = hdr[4] == 2;
		bool is32 = hdr[4] == 1;
		if (!is64 && !is32) throw std::runtime_error("unsupported ELF class");
		uint64_t phoff = is64 ? rd64(hdr + 32) : rd32(hdr + 28);
		uint16_t phentsize = is64 ? rd16(hdr + 54) : rd16(hdr + 42);
		uint16_t phnum = is64 ? rd16(hdr + 56) : rd16(hdr + 44);

		for (uint16_t i = 0; i < phnum; ++i) {
			uint64_t off = phoff + (uint64_t)i * phentsize;
			if (off + phentsize > elf.size()) throw std::runtime_error("ELF phdr out of range");
			const uint8_t *ph = elf.data() + off;
			uint32_t p_type = rd32(ph + 0);
			if (p_type != 1u) continue;
			uint64_t p_offset = is64 ? rd64(ph + 8) : rd32(ph + 4);
			uint64_t p_paddr = is64 ? rd64(ph + 24) : rd32(ph + 12);
			uint64_t p_filesz = is64 ? rd64(ph + 32) : rd32(ph + 16);
			if (p_offset + p_filesz > elf.size()) throw std::runtime_error("ELF segment out of range");
			for (uint64_t j = 0; j < p_filesz; j += 4) {
				uint32_t data = 0;
				uint32_t strb = 0;
				for (uint32_t b = 0; b < 4 && (j + b) < p_filesz; ++b) {
					data |= (uint32_t)elf[p_offset + j + b] << (8 * b);
					strb |= 1u << b;
				}
				bus_write((uint32_t)(p_paddr + j), data, strb);
			}
		}
	}

	BusStats stats() const { return stats_; }

private:
	static bool in_ram(uint32_t addr)
	{
		return addr >= 0x80000000u && addr < 0x80020000u;
	}

	void drive_idle()
	{
		tb_->mem_valid = 0;
		tb_->mem_wstrb = 0;
		tb_->mem_wdata = 0;
	}

	uint32_t bus_read(uint32_t addr)
	{
		stats_.rd_total++;
		if (in_ram(addr)) stats_.rd_ram++;
		tb_->mem_valid = 1;
		tb_->mem_instr = 0;
		tb_->mem_addr = addr;
		tb_->mem_wstrb = 0;
		tb_->mem_wdata = 0;
		do {
			tick();
		} while (!tb_->mem_ready);
		const uint32_t r = tb_->mem_rdata;
		drive_idle();
		tick();
		return r;
	}

	void bus_write(uint32_t addr, uint32_t data, uint32_t wstrb)
	{
		stats_.wr_total++;
		if (in_ram(addr)) stats_.wr_ram++;
		tb_->mem_valid = 1;
		tb_->mem_instr = 0;
		tb_->mem_addr = addr;
		tb_->mem_wdata = data;
		tb_->mem_wstrb = wstrb & 0xF;
		do {
			tick();
		} while (!tb_->mem_ready);
		drive_idle();
		tick();
	}

	Vpicosoc_soc_bus *tb_;
	BusStats stats_{};
};

std::string env_or_default(const char *name, const char *fallback)
{
	const char *v = std::getenv(name);
	if (!v || v[0] == '\0') return std::string(fallback);
	return std::string(v);
}

bool env_enabled(const char *name)
{
	const char *v = std::getenv(name);
	if (!v || v[0] == '\0') return false;
	return std::strcmp(v, "0") != 0;
}

} // namespace

int main(int argc, char **argv)
{
	Verilated::commandArgs(argc, argv);

	const std::string elf_path = env_or_default("TEST_ELF", "build/firmware/hello/obj/firmware.elf");
	const std::string case_name = env_or_default("SOC_CASE_NAME", "hello");
	const std::string isa = env_or_default("MY_ISA", "RV32IMC");
	const std::string commit_log_path = env_or_default("SOC_SPIKE_COMMIT_LOG", "");
	const uint64_t max_steps = 2000000;
	const bool verbose = env_enabled("SOC_SPIKE_VERBOSE");

	auto tb = std::make_unique<Vpicosoc_soc_bus>();
	auto simif = std::make_unique<TbBusSimif>(tb.get());
	simif->reset();

	std::unique_ptr<FILE, int (*)(FILE*)> commit_log(nullptr, &std::fclose);
	if (!commit_log_path.empty()) {
		FILE *f = std::fopen(commit_log_path.c_str(), "w");
		if (!f) {
			throw std::runtime_error("failed to open commit log: " + commit_log_path);
		}
		commit_log.reset(f);
	}

	auto isa_parser = std::make_unique<isa_parser_t>(isa.c_str(), DEFAULT_PRIV);
	processor_t cpu(isa_parser.get(), DEFAULT_VARCH, simif.get(), 0, false, commit_log.get(), std::cerr);
	if (!commit_log_path.empty()) {
		cpu.set_debug(true);
#ifdef RISCV_ENABLE_COMMITLOG
		cpu.enable_log_commits();
#endif
	}
	cpu.reset();
	simif->preload_elf(elf_path);
	const reg_t entry = static_cast<reg_t>(read_elf_entry(elf_path));
	cpu.get_state()->pc = entry;
	cpu.put_csr(0x305, entry); // mtvec
	cpu.put_csr(0x304, 0);     // mie
	cpu.put_csr(0x300, 0);     // mstatus
	if (verbose) {
		try {
			const uint32_t insn0 = cpu.get_mmu()->load_uint32(entry);
			std::cerr << "[soc-spike] insn@entry=0x" << std::hex << insn0 << std::dec << "\n";
		} catch (trap_t &t) {
			std::cerr << "[soc-spike] load entry trap: " << t.name() << "\n";
		}
		std::cerr << "[soc-spike] entry=0x" << std::hex << static_cast<uint32_t>(entry)
		          << " pc0=0x" << static_cast<uint32_t>(cpu.get_state()->pc) << std::dec << "\n";
	}
	std::cout << "[SOC-SPIKE] case=" << case_name << "\n";

	for (uint64_t i = 0; i < max_steps; ++i) {
		try {
			cpu.step(1);
		} catch (trap_t &t) {
			std::cerr << "trap: " << t.name() << "\n";
			return 2;
		}
		if (verbose && (i % 100000) == 0) {
			std::cerr << "[soc-spike] step=" << i
			          << " pc=0x" << std::hex << static_cast<uint32_t>(cpu.get_state()->pc)
			          << std::dec << "\n";
		}

		if (tb->finish_seen) break;
	}

	if (!tb->finish_seen) {
		std::cerr << "FAIL: pass marker not observed via MMIO 0x20000000, final pc=0x"
		          << std::hex << static_cast<uint32_t>(cpu.get_state()->pc) << std::dec << "\n";
		return 3;
	}

	const auto st = simif->stats();
	std::cout << "[SOC-BUS] rd_total=" << st.rd_total
	          << " rd_ram=" << st.rd_ram
	          << " wr_total=" << st.wr_total
	          << " wr_ram=" << st.wr_ram << "\n";
	if (st.rd_ram == 0) {
		std::cerr << "FAIL: no RAM reads observed on SoC bus (0x80000000 region)\n";
		return 4;
	}

	std::cout << "PASS: spike drove picosoc shell bus and observed finish marker\n";
	return 0;
}
