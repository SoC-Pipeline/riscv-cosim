#include "cosim_config_policy.h"

#include <cstdlib>
#include <stdexcept>

namespace cosim {

namespace {

std::string EnvOrEmpty(const char* name)
{
    if (name == nullptr || name[0] == '\0') {
        return "";
    }
    const char* value = std::getenv(name);
    return (value == nullptr || value[0] == '\0') ? "" : std::string(value);
}

std::string ResolveLogPath(const char* generic_env, const char* target_env, const std::string& fallback)
{
    const std::string generic = EnvOrEmpty(generic_env);
    if (!generic.empty()) {
        return generic;
    }
    const std::string target = EnvOrEmpty(target_env);
    if (!target.empty()) {
        return target;
    }
    return fallback;
}

std::string ResolveCommitLogPath(const char* generic_env, const char* target_env,
                                 const char* legacy_target_env, const std::string& fallback)
{
    const std::string generic = EnvOrEmpty(generic_env);
    if (!generic.empty()) {
        return generic;
    }
    const std::string target = EnvOrEmpty(target_env);
    if (!target.empty()) {
        return target;
    }
    const std::string legacy_target = EnvOrEmpty(legacy_target_env);
    if (!legacy_target.empty()) {
        return legacy_target;
    }
    return fallback;
}

} // namespace

CosimConfig BuildCosimConfig(const CosimPolicyArgs& args,
                             const char* target_cosim_env,
                             const char* target_commit_env,
                             const char* legacy_commit_env)
{
    if (args.cpu_name.empty()) {
        throw std::runtime_error("cpu_name is required");
    }
    if (args.elf_path.empty()) {
        throw std::runtime_error("elf_path is required");
    }

    CosimConfig config;
    config.elf_path = args.elf_path;
    config.isa = args.isa;
    config.memory_base = args.memory_base;
    config.memory_size = args.memory_size;
    config.dtb_enabled = args.dtb_enabled;
    config.dtb_file = args.dtb_file;
    config.sim_mmio_enabled = args.sim_mmio_enabled;
    config.log_path = ResolveLogPath("COSIM_LOG_PATH", target_cosim_env,
                                     "dump/" + args.cpu_name + "_cosim_result.log");
    config.commit_log_path = ResolveCommitLogPath("SPIKE_COMMIT_LOG_PATH", target_commit_env,
                                                  legacy_commit_env,
                                                  "dump/" + args.cpu_name + "_spike_commit.log");
    return config;
}

} // namespace cosim
