#include "spike_simulator.h"

#include "elf_utils.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <riscv/common.h>
#include <riscv/config.h>
#include <riscv/devices.h>
#include <riscv/isa_parser.h>
#include <riscv/mmu.h>
#include <riscv/processor.h>
#include <riscv/simif.h>
#include <riscv/trap.h>

namespace {

uint16_t read_u16_le(const uint8_t* p)
{
    return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}

uint32_t read_u32_le(const uint8_t* p)
{
    return static_cast<uint32_t>(p[0]) |
           (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

uint64_t read_u64_le(const uint8_t* p)
{
    return static_cast<uint64_t>(read_u32_le(p)) |
           (static_cast<uint64_t>(read_u32_le(p + 4)) << 32);
}

void load_elf_segments(mem_t& mem, uint64_t memory_base, const std::string& elf_path)
{
    FILE* file = std::fopen(elf_path.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("failed to open ELF: " + elf_path);
    }

    auto close_file = std::unique_ptr<FILE, int (*)(FILE*)>(file, &std::fclose);
    std::vector<uint8_t> elf_data;
    std::fseek(file, 0, SEEK_END);
    long file_size = std::ftell(file);
    if (file_size <= 0) {
        throw std::runtime_error("invalid ELF size: " + elf_path);
    }
    std::rewind(file);
    elf_data.resize(static_cast<size_t>(file_size));
    if (std::fread(elf_data.data(), 1, elf_data.size(), file) != elf_data.size()) {
        throw std::runtime_error("failed to read ELF: " + elf_path);
    }

    const uint8_t* hdr = elf_data.data();
    if (elf_data.size() < 0x40 || hdr[0] != 0x7f || hdr[1] != 'E' || hdr[2] != 'L' || hdr[3] != 'F') {
        throw std::runtime_error("invalid ELF header: " + elf_path);
    }
    if (hdr[5] != 1) {
        throw std::runtime_error("unsupported non-little-endian ELF: " + elf_path);
    }

    const bool is_elf64 = (hdr[4] == 2);
    const bool is_elf32 = (hdr[4] == 1);
    if (!is_elf32 && !is_elf64) {
        throw std::runtime_error("unsupported ELF class: " + elf_path);
    }

    uint64_t phoff = is_elf64 ? read_u64_le(hdr + 32) : read_u32_le(hdr + 28);
    uint16_t phentsize = is_elf64 ? read_u16_le(hdr + 54) : read_u16_le(hdr + 42);
    uint16_t phnum = is_elf64 ? read_u16_le(hdr + 56) : read_u16_le(hdr + 44);

    for (uint16_t i = 0; i < phnum; ++i) {
        uint64_t off = phoff + static_cast<uint64_t>(i) * phentsize;
        if (off + phentsize > elf_data.size()) {
            throw std::runtime_error("program header out of range in ELF: " + elf_path);
        }

        const uint8_t* ph = elf_data.data() + off;
        uint32_t p_type = read_u32_le(ph + 0);
        if (p_type != 1u) {
            continue;
        }

        uint64_t p_offset = is_elf64 ? read_u64_le(ph + 8) : read_u32_le(ph + 4);
        uint64_t p_paddr = is_elf64 ? read_u64_le(ph + 24) : read_u32_le(ph + 12);
        uint64_t p_filesz = is_elf64 ? read_u64_le(ph + 32) : read_u32_le(ph + 16);
        uint64_t p_memsz = is_elf64 ? read_u64_le(ph + 40) : read_u32_le(ph + 20);

        if (p_offset + p_filesz > elf_data.size()) {
            throw std::runtime_error("PT_LOAD data out of range in ELF: " + elf_path);
        }
        if (p_paddr < memory_base) {
            continue;
        }

        uint64_t mem_off = p_paddr - memory_base;
        if (mem_off + p_memsz > mem.size()) {
            continue;
        }

        if (p_filesz > 0) {
            std::memcpy(mem.contents(static_cast<reg_t>(mem_off)), elf_data.data() + p_offset,
                        static_cast<size_t>(p_filesz));
        }
        if (p_memsz > p_filesz) {
            std::memset(mem.contents(static_cast<reg_t>(mem_off + p_filesz)), 0,
                        static_cast<size_t>(p_memsz - p_filesz));
        }
    }
}

class SimulationMmioDevice : public abstract_device_t {
public:
    explicit SimulationMmioDevice(uint32_t finish_value)
        : finish_value_(finish_value)
    {
    }

    bool load(reg_t addr, size_t len, uint8_t* bytes) override
    {
        if (addr != 0 || len > sizeof(uint64_t)) {
            return false;
        }
        std::memset(bytes, 0, len);
        return true;
    }

    bool store(reg_t addr, size_t len, const uint8_t* bytes) override
    {
        if (addr != 0 || len == 0 || len > sizeof(uint64_t)) {
            return false;
        }
        uint64_t value = 0;
        for (size_t i = 0; i < len; ++i) {
            value |= static_cast<uint64_t>(bytes[i]) << (8 * i);
        }
        last_store_ = value;
        if (static_cast<uint32_t>(value) == finish_value_) {
            finished_ = true;
        }
        return true;
    }

    bool finished() const { return finished_; }

private:
    uint32_t finish_value_;
    uint64_t last_store_ = 0;
    bool finished_ = false;
};

class SpikeSimIf final : public simif_t {
public:
    void add_memory(reg_t base, mem_t* mem)
    {
        bus_.add_device(base, mem);
    }

    void add_mmio(reg_t base, abstract_device_t* dev)
    {
        bus_.add_device(base, dev);
    }

    char* addr_to_mem(reg_t addr) override
    {
        auto desc = bus_.find_device(addr);
        auto* mem = dynamic_cast<mem_t*>(desc.second);
        if (!mem) {
            return nullptr;
        }
        reg_t off = addr - desc.first;
        if (off >= mem->size()) {
            return nullptr;
        }
        return mem->contents(off);
    }

    bool mmio_load(reg_t addr, size_t len, uint8_t* bytes) override
    {
        return bus_.load(addr, len, bytes);
    }

    bool mmio_store(reg_t addr, size_t len, const uint8_t* bytes) override
    {
        return bus_.store(addr, len, bytes);
    }

    void proc_reset(unsigned id) override { (void)id; }

    const char* get_symbol(uint64_t addr) override
    {
        (void)addr;
        return nullptr;
    }

private:
    bus_t bus_;
};

} // namespace

struct SpikeSimulator::Impl {
    processor_t& cpu() const
    {
        if (!cpu_) {
            throw std::runtime_error("Spike not initialized");
        }
        return *cpu_;
    }

    std::unique_ptr<SpikeSimIf> simif;
    std::unique_ptr<isa_parser_t> isa_parser;
    std::unique_ptr<processor_t> cpu_;
    std::unique_ptr<FILE, int (*)(FILE*)> log_file{nullptr, &std::fclose};
    std::vector<std::unique_ptr<mem_t>> owned_mems;
    std::vector<std::unique_ptr<abstract_device_t>> owned_devices;
    SimulationMmioDevice* finish_device = nullptr;
};

SpikeSimulator::SpikeSimulator()
    : impl_(std::make_unique<Impl>())
{
}

SpikeSimulator::~SpikeSimulator() = default;

void SpikeSimulator::init(const CosimConfig& config)
{
    impl_->cpu_.reset();
    impl_->isa_parser.reset();
    impl_->simif.reset();
    impl_->owned_mems.clear();
    impl_->owned_devices.clear();
    impl_->finish_device = nullptr;
    impl_->log_file.reset(nullptr);

    if (!config.commit_log_path.empty()) {
        FILE* file = std::fopen(config.commit_log_path.c_str(), "w");
        if (!file) {
            throw std::runtime_error("failed to open commit log file: " + config.commit_log_path);
        }
        impl_->log_file.reset(file);
    }

    impl_->simif = std::make_unique<SpikeSimIf>();
    impl_->owned_mems.push_back(std::make_unique<mem_t>(config.memory_size));
    impl_->simif->add_memory(static_cast<reg_t>(config.memory_base), impl_->owned_mems.back().get());
    load_elf_segments(*impl_->owned_mems.back(), config.memory_base, config.elf_path);

    if (config.sim_mmio_enabled) {
        impl_->owned_devices.push_back(std::make_unique<SimulationMmioDevice>(config.sim_finish_value));
        impl_->simif->add_mmio(static_cast<reg_t>(config.sim_print_addr), impl_->owned_devices.back().get());
        impl_->owned_devices.push_back(std::make_unique<SimulationMmioDevice>(config.sim_finish_value));
        impl_->finish_device = static_cast<SimulationMmioDevice*>(impl_->owned_devices.back().get());
        impl_->simif->add_mmio(static_cast<reg_t>(config.sim_finish_addr), impl_->owned_devices.back().get());
    }

    impl_->isa_parser = std::make_unique<isa_parser_t>(config.isa.c_str(), DEFAULT_PRIV);
    impl_->cpu_ = std::make_unique<processor_t>(impl_->isa_parser.get(), DEFAULT_VARCH, impl_->simif.get(),
                                                0, false, impl_->log_file.get(), std::cerr);
    impl_->cpu_->reset();
    impl_->cpu_->get_state()->pc = static_cast<reg_t>(read_elf_entry(config.elf_path));

    if (!config.commit_log_path.empty()) {
        impl_->cpu_->set_debug(true);
#ifdef RISCV_ENABLE_COMMITLOG
        impl_->cpu_->enable_log_commits();
#endif
    }
}

void SpikeSimulator::step(uint64_t count)
{
    try {
        impl_->cpu().step(static_cast<size_t>(count));
    } catch (mem_trap_t& trap) {
        std::ostringstream message;
        message << "access exception while running Spike at 0x" << std::hex << trap.get_tval();
        throw std::runtime_error(message.str());
    }
}

uint64_t SpikeSimulator::pc() const
{
    return impl_->cpu().get_state()->pc;
}

uint32_t SpikeSimulator::instruction_at(uint64_t pc)
{
    mmu_t* mmu = impl_->cpu().get_mmu();
    if (!mmu) {
        throw std::runtime_error("failed to get Spike MMU");
    }

    try {
        insn_fetch_t fetched = mmu->load_insn(static_cast<reg_t>(pc));
        return fetched.insn.bits();
    } catch (trap_t&) {
        return 0xffffffff;
    }
}

uint64_t SpikeSimulator::reg(unsigned index) const
{
    if (index >= 32) {
        throw std::out_of_range("invalid register index");
    }
    return impl_->cpu().get_state()->XPR[index];
}

void SpikeSimulator::set_csr(unsigned csr_num, uint32_t value)
{
    impl_->cpu().put_csr(static_cast<int>(csr_num), value);
}

uint32_t SpikeSimulator::get_csr(unsigned csr_num)
{
    return static_cast<uint32_t>(impl_->cpu().get_csr(static_cast<int>(csr_num)));
}

void SpikeSimulator::set_mip(uint32_t mip)
{
    impl_->cpu().get_state()->mip->write_with_mask(0xffffffffu, mip);
    impl_->cpu().get_state()->mip->write_pre_val(mip);
}

void SpikeSimulator::set_nmi(bool nmi)
{
    impl_->cpu().get_state()->nmi = nmi;
}

void SpikeSimulator::set_debug_req(bool debug_req)
{
    impl_->cpu().halt_request = debug_req ? processor_t::HR_REGULAR : processor_t::HR_NONE;
}

bool SpikeSimulator::finished() const
{
    return impl_->finish_device != nullptr && impl_->finish_device->finished();
}

const char* SpikeSimulator::name() const
{
    return "spike";
}
