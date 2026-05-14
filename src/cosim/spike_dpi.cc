#include <cstring>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <sys/syscall.h>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>

#include "cosim_bridge.h"
#include "cosim_config_policy.h"

#ifdef VPI_WRAPPER
    #include "vpi_user.h"
#elif DPI_WRAPPER
    #include "svdpi.h"
#endif

static std::string env_string(const char* name, const char* default_value)
{
    const char* value = std::getenv(name);
    return value == nullptr || value[0] == '\0' ? std::string(default_value) : std::string(value);
}

#ifdef VPI_WRAPPER
static std::string get_plusarg_value(const char* prefix)
{
    s_vpi_vlog_info vlog_info;
    if (!vpi_get_vlog_info(&vlog_info)) {
        return "";
    }

    size_t prefix_len = std::strlen(prefix);
    for (int i = 0; i < vlog_info.argc; ++i) {
        const char* arg = vlog_info.argv[i];
        if (arg && std::strncmp(arg, prefix, prefix_len) == 0) {
            return std::string(arg + prefix_len);
        }
    }
    return "";
}

static bool session_ready(const char* task_name)
{
    if (!cosim_bridge_initialized()) {
        vpi_printf("[VPI ERROR] %s: cosim not initialized\n", task_name);
        return false;
    }
    return true;
}

static uint32_t get_u32_arg(vpiHandle arg_h)
{
    s_vpi_value arg_value;
    arg_value.format = vpiVectorVal;
    vpi_get_value(arg_h, &arg_value);
    return arg_value.value.vector[0].aval;
}

static void put_u32_value(vpiHandle arg_h, uint32_t value)
{
    s_vpi_vecval vec;
    vec.aval = value;
    vec.bval = 0;

    s_vpi_value arg_value;
    arg_value.format = vpiVectorVal;
    arg_value.value.vector = &vec;
    vpi_put_value(arg_h, &arg_value, NULL, vpiNoDelay);
}

static void register_task(const char* name, PLI_INT32 (*calltf)(PLI_BYTE8*))
{
    s_vpi_systf_data tf_data;
    tf_data.type = vpiSysTask;
    tf_data.sysfunctype = 0;
    tf_data.calltf = calltf;
    tf_data.compiletf = NULL;
    tf_data.sizetf = NULL;
    tf_data.user_data = NULL;
    tf_data.tfname = const_cast<char*>(name);
    vpi_register_systf(&tf_data);
}
#endif

extern "C" {
#ifdef VPI_WRAPPER
    PLI_INT32 cosim_init_vpi_calltf(PLI_BYTE8 *user_data) {
        vpiHandle systf_handle = vpi_handle(vpiSysTfCall, NULL);
        vpiHandle arg_iterator = vpi_iterate(vpiArgument, systf_handle);
        if (!arg_iterator) {
            vpi_printf("[VPI ERROR] $cosim_init: invalid handle.\n");
            return 1;
        }

        std::string elf_path = get_plusarg_value("+ELF_PATH=");
        if (elf_path.empty()) {
            vpiHandle arg_handle = vpi_scan(arg_iterator);
            if (!arg_handle) {
                vpi_printf("[VPI ERROR] $cosim_init: requires an ELF path argument\n");
                vpi_free_object(arg_iterator);
                return 1;
            }
            s_vpi_value arg_value;
            arg_value.format = vpiStringVal;
            vpi_get_value(arg_handle, &arg_value);
            elf_path = arg_value.value.str;
        }

        vpi_free_object(arg_iterator);

        const char* init_stage = "begin";
        try {
            const CosimConfig config = cosim::BuildCosimConfig(
                cosim::CosimPolicyArgs{
                    .cpu_name = "picorv32",
                    .elf_path = elf_path,
                    .isa = "RV32IMC",
                    .memory_base = 0x80000000,
                    .memory_size = 128 * 1024,
                    .dtb_enabled = false,
                    .sim_mmio_enabled = true
                },
                "PICORV32_COSIM_LOG", "PICORV32_SPIKE_COMMIT_LOG");

            init_stage = "ELF file check";
            std::ifstream elf_file_chk(elf_path, std::ios::ate | std::ios::binary);
            if (!elf_file_chk.is_open()) {
                std::cerr << "[VPI ERROR] $cosim_init - failed to open ELF: " << elf_path << std::endl;
                return 1;
            }
            std::cout << "[VPI INFO] $cosim_init - ELF load success(" << elf_path << ")" << std::endl;

            init_stage = "session init";
            if (cosim_bridge_init(&config) != 0) {
                throw std::runtime_error("failed to initialize cosim bridge");
            }
        } catch (const std::exception& e) {
            vpi_printf("[VPI ERROR] $cosim_init: exception at %s: %s\n", init_stage, e.what());
            cosim_bridge_reset();
            return 1;
        }

        return 0;
    }

    PLI_INT32 cosim_retire_vpi_calltf(PLI_BYTE8* user_data) {
        if (!session_ready("$cosim_retire")) {
            return 1;
        }

        vpiHandle systf_handle = vpi_handle(vpiSysTfCall, NULL);
        vpiHandle args_iter = vpi_iterate(vpiArgument, systf_handle);
        if (!args_iter) {
            vpi_printf("[VPI ERROR] $cosim_retire: requires PC and instruction arguments\n");
            return 1;
        }

        vpiHandle pc_arg = vpi_scan(args_iter);
        vpiHandle instr_arg = vpi_scan(args_iter);
        if (!pc_arg || !instr_arg) {
            vpi_printf("[VPI ERROR] $cosim_retire: requires PC and instruction arguments\n");
            vpi_free_object(args_iter);
            return 1;
        }

        uint32_t dut_pc = get_u32_arg(pc_arg);
        uint32_t dut_instr = get_u32_arg(instr_arg);
        vpi_free_object(args_iter);

        try {
            if (cosim_bridge_retire(dut_pc, dut_instr) != 0) {
                throw std::runtime_error("bridge retire failed");
            }
        } catch (const std::exception& e) {
            vpi_printf("[VPI ERROR] $cosim_retire: %s\n", e.what());
            return 1;
        }

        return 0;
    }

    PLI_INT32 cosim_finish_vpi_calltf(PLI_BYTE8* user_data) {
        if (!cosim_bridge_initialized()) {
            return 0;
        }

        if (cosim_bridge_finish() != 0) {
            vpi_printf("[VPI ERROR] $cosim_finish: bridge finish failed\n");
            return 1;
        }

        return 0;
    }

    PLI_INT32 cosim_get_status_vpi_calltf(PLI_BYTE8* user_data) {
        if (!session_ready("$cosim_get_status")) {
            return 1;
        }

        vpiHandle systf_handle = vpi_handle(vpiSysTfCall, NULL);
        vpiHandle args_iter = vpi_iterate(vpiArgument, systf_handle);
        if (!args_iter) {
            vpi_printf("[VPI ERROR] $cosim_get_status: requires pass and fail output arguments\n");
            return 1;
        }

        vpiHandle pass_arg = vpi_scan(args_iter);
        vpiHandle fail_arg = vpi_scan(args_iter);
        if (!pass_arg || !fail_arg) {
            vpi_printf("[VPI ERROR] $cosim_get_status: requires pass and fail output arguments\n");
            vpi_free_object(args_iter);
            return 1;
        }

        put_u32_value(pass_arg, static_cast<uint32_t>(cosim_bridge_pass_count()));
        put_u32_value(fail_arg, static_cast<uint32_t>(cosim_bridge_fail_count()));
        vpi_free_object(args_iter);

        return 0;
    }

    PLI_INT32 cleanup_cosim_vpi(p_cb_data cb_data_p) {
        if (cosim_bridge_initialized()) {
            cosim_bridge_reset();
            std::cout << "[VPI_INFO] Cosim session cleaned up." << std::endl;
        }
        return 0;
    }

    void register_cosim_vpi_tasks() {
        register_task("$cosim_init", cosim_init_vpi_calltf);
        register_task("$cosim_retire", cosim_retire_vpi_calltf);
        register_task("$cosim_finish", cosim_finish_vpi_calltf);
        register_task("$cosim_get_status", cosim_get_status_vpi_calltf);

        s_cb_data cb_data_end;
        cb_data_end.reason = cbEndOfSimulation;
        cb_data_end.cb_rtn = cleanup_cosim_vpi;
        cb_data_end.obj = NULL;
        cb_data_end.time = NULL;
        cb_data_end.value = NULL;
        cb_data_end.user_data = NULL;
        vpi_register_cb(&cb_data_end);
    }

    void (*vlog_startup_routines[])() = {
        register_cosim_vpi_tasks,
        0
    };
#elif DPI_WRAPPER
    void cosim_dpi_init() {
        const CosimConfig config = cosim::BuildCosimConfig(
            cosim::CosimPolicyArgs{
                .cpu_name = "picorv32",
                .elf_path = env_string("TEST_ELF", "build/firmware/hello/obj/firmware.elf"),
                .isa = env_string("MY_ISA", "RV32IMC"),
                .memory_base = 0x80000000,
                .memory_size = 128 * 1024,
                .dtb_enabled = false,
                .sim_mmio_enabled = true
            },
            "PICORV32_COSIM_LOG", "PICORV32_SPIKE_COMMIT_LOG");
        (void)cosim_bridge_init(&config);
    }

    void cosim_dpi_retire(uint32_t pc, uint32_t instr) {
        (void)cosim_bridge_retire(pc, instr);
    }

    void cosim_dpi_finish() {
        (void)cosim_bridge_finish();
    }

    uint32_t cosim_dpi_fail_count() {
        return static_cast<uint32_t>(cosim_bridge_fail_count());
    }

    void cosim_dpi_reset() {
        cosim_bridge_reset();
    }
#else
    std::cerr << "[Cosim] ERROR: DPI/VPI interface is not defined. Add the option under compiling with g++\n";
#endif
}
