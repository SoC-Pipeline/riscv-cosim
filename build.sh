#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

set_default() {
	local name="$1"
	local value="$2"
	if [[ -z "${!name:-}" ]]; then
		printf -v "$name" '%s' "$value"
	fi
	export "$name"
}

set_default TB_HOME "$SCRIPT_DIR"
set_default TEST_NAME hello
set_default SRC_DIR "$TB_HOME/src"
set_default SRC_TOP_DIR "$SRC_DIR/top_cpu"
set_default COSIM_SRC_DIR "$SRC_DIR/cosim"
set_default PICORV32_DIR "$TB_HOME/external/picorv32"
set_default PICORV32_RTL "$PICORV32_DIR/picorv32.v"
set_default IBEX_DIR "$TB_HOME/external/ibex"
set_default VEER_EL2_DIR "$TB_HOME/external/Cores-VeeR-EL2"
set_default SPIKE_SRC_DIR "$IBEX_DIR/external/riscv-isa-sim"
set_default PK_SRC_DIR "$TB_HOME/external/riscv-pk"
set_default FIRMWARE_DIR "$TB_HOME/firmware"
set_default FIRMWARE_COMMON_DIR "$FIRMWARE_DIR/common"
set_default SCRIPTS_DIR "$TB_HOME/scripts"
set_default BUILD_DIR "$TB_HOME/build"
set_default SPIKE_BUILD_DIR "$BUILD_DIR/spike-build"
set_default SPIKE_PREFIX "$BUILD_DIR/spike"
set_default PK_BUILD_DIR "$BUILD_DIR/pk-build"
set_default PK_PREFIX "$BUILD_DIR/pk"
set_default FIRMWARE_BUILD_DIR "$BUILD_DIR/firmware"
set_default SRC_BUILD_DIR "$BUILD_DIR/src"
set_default TOP_BUILD_DIR "$SRC_BUILD_DIR/top_cpu"
set_default COSIM_BUILD_DIR "$SRC_BUILD_DIR/cosim"
set_default IBEX_BUILD_ROOT "$TOP_BUILD_DIR/ibex"
set_default VEER_EL2_BUILD_ROOT "$TOP_BUILD_DIR/veer_el2"
set_default VEER_EL2_CONFIG_DIR "$VEER_EL2_BUILD_ROOT/snapshots/default"
set_default VEER_EL2_FLIST "$VEER_EL2_BUILD_ROOT/flist"
set_default VEER_EL2_OBJ_DIR "$VEER_EL2_BUILD_ROOT/obj_dir"
set_default VEER_EL2_SIM_EXE "$VEER_EL2_OBJ_DIR/Vtb_top"
set_default VEER_EL2_BUILD_STAMP "$VEER_EL2_BUILD_ROOT/build.stamp"
set_default VEER_EL2_SPIKE_DTS "$VEER_EL2_BUILD_ROOT/spike_veer_el2.dts"
set_default VEER_EL2_SPIKE_DTB "$VEER_EL2_BUILD_ROOT/spike_veer_el2.dtb"
set_default LOG_DIR "$TB_HOME/log"
set_default DUMP_DIR "$LOG_DIR"
set_default VPI_MODULE_DIR "$COSIM_BUILD_DIR"
set_default SIM_VVP "$TOP_BUILD_DIR/tb_picorv32.vvp"
set_default PICORV32_TOP_STAMP "$TOP_BUILD_DIR/picorv32_top.stamp"

set_default RISCV_ROOT "${RISCV:-${RISCV_PATH:-${RISCV_TOOLCHAIN:-${RISCV_GNU_TOOLCHAIN_INSTALL_PREFIX:-}}}}"
set_default SPIKE_ROOT "$SPIKE_PREFIX"
if [[ -n "$RISCV_ROOT" ]]; then
	set_default TOOLCHAIN_PREFIX "$RISCV_ROOT/bin/riscv32-unknown-elf-"
else
	set_default TOOLCHAIN_PREFIX riscv32-unknown-elf-
fi

PK_HOST_DEFAULT="${TOOLCHAIN_PREFIX##*/}"
PK_HOST_DEFAULT="${PK_HOST_DEFAULT%-}"

set_default MY_ISA RV32IMC
set_default RESET_VECTOR 0x80000080
set_default PK_ARCH rv32ic_zicsr_zifencei
set_default PK_HOST "$PK_HOST_DEFAULT"
set_default MY_PK_PATH "$PK_PREFIX/riscv32-unknown-elf/bin/pk"
set_default IBEX_PKG_CONFIG_PATH "$SPIKE_PREFIX/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"
set_default IBEX_FUSESOC_CORE local:spike_cosim:tb_ibex_cosim
set_default IBEX_BOOT_ADDR "$(( (RESET_VECTOR - 0x80) & 0xFFFFFFFF ))"
set_default IBEX_RAM_BASE 2147483648
set_default PICORV32_COSIM_LOG "$LOG_DIR/picorv32_cosim_result.log"
set_default PICORV32_SPIKE_COMMIT_LOG "$LOG_DIR/picorv32_spike_commit.log"
set_default PICORV32_COSIM_IF vpi
set_default IBEX_COSIM_LOG "$LOG_DIR/ibex_cosim_result.log"
set_default IBEX_SPIKE_COMMIT_LOG "$LOG_DIR/ibex_spike_commit.log"
set_default IBEX_TRACE_FILE_BASE "$LOG_DIR/trace_core"
set_default IBEX_SIMPLE_SYSTEM_LOG "$LOG_DIR/ibex_simple_system.log"
set_default IBEX_PCOUNT_CSV "$LOG_DIR/tb_ibex_pcount.csv"
set_default IBEX_VERILATOR_VERSION 5.010
set_default VEER_EL2_RAM_BASE 0x80000000
set_default VEER_EL2_RAM_SIZE 0x00020000
set_default VEER_EL2_MAX_CYCLES 2000000
set_default VEER_EL2_COSIM_LOG "$LOG_DIR/veer_el2_cosim_result.log"
set_default VEER_EL2_SPIKE_COMMIT_LOG "$LOG_DIR/veer_el2_spike_commit.log"
set_default FIRMWARE_CASES "hello pico_test"

set_default PYTHON python3
set_default CXX g++
set_default IVERILOG "iverilog${ICARUS_SUFFIX:-}"
set_default VVP "vvp${ICARUS_SUFFIX:-}"
set_default FUSESOC fusesoc
set_default PKG_CONFIG pkg-config
set_default BUILD_JOBS 8
set_default SPIKE_LIB_DIR "$SPIKE_ROOT/lib"

SPIKE_INCLUDE_DIRS=(
	"$SPIKE_ROOT/include"
	"$SPIKE_ROOT/include/riscv"
	"$SPIKE_ROOT/include/fesvr"
	"$SPIKE_ROOT/include/softfloat"
)
VPI_INCLUDE_DIRS=()
for dir in /usr/include/iverilog /usr/local/include/iverilog /usr/share/verilator/include/vltstd /usr/local/share/verilator/include/vltstd; do
	[[ -d "$dir" ]] && VPI_INCLUDE_DIRS+=("$dir")
done

usage() {
	cat <<'EOF'
Usage:
  ./build.sh <command> [args]

Common flows:
  ./build.sh build spike
      Build the shared local dependencies:
        build/spike, build/pk, build/src/cosim/libspike.vpi

  ./build.sh build -f [case|all]
      Build firmware ELF/HEX artifacts under build/firmware/<case>/obj.
      Default case selector: all

  ./build.sh build -t [target|all]
      Build simulation top artifacts.
      Default target selector: all

  ./build.sh run [target|all] [case|all]
      Ensure missing dependencies, firmware, and top artifacts, then run the
      selected simulation matrix.
      Defaults: target=all, case=$TEST_NAME

  ./build.sh all
      Shortcut for: ./build.sh run all all

Selectors:
  target: picorv32, ibex, veer_el2, all
  case:   hello, pico_test, all

Examples:
  ./build.sh
  ./build.sh build spike
  ./build.sh build -f hello
  ./build.sh build -t ibex
  ./build.sh run picorv32 hello
  ./build.sh run ibex pico_test
  ./build.sh run veer_el2 hello
  ./build.sh run all all

Clean:
  ./build.sh clean
      Remove generated firmware, top, cosim, cache, and log outputs.
      Preserve slow dependency builds:
        build/spike, build/spike-build, build/pk, build/pk-build

  ./build.sh clean-all
      Remove the full build directory plus log outputs.

Frequently used environment overrides:
  FIRMWARE_CASES      Active case list used by all/run all all.
                      Default: hello pico_test
  RESET_VECTOR        Shared firmware reset vector. Default: 0x80000080
  LOG_DIR             Simulation log output directory. Default: log
  COSIM_LOG_PATH      Generic cosim compare log path override for all targets.
  SPIKE_COMMIT_LOG_PATH
                      Generic Spike commit log path override for all targets.
                      Priority:
                        1) COSIM_LOG_PATH/SPIKE_COMMIT_LOG_PATH
                        2) target-specific vars below
                        3) log/<cpu_name>_cosim_result.log and
                           log/<cpu_name>_spike_commit.log
  PICORV32_COSIM_LOG  PicoRV32 cosim compare log. Default: log/picorv32_cosim_result.log
  PICORV32_SPIKE_COMMIT_LOG
                      PicoRV32 Spike commit log. Default: log/picorv32_spike_commit.log
  BUILD_JOBS          Parallel jobs for Spike/pk/top builds. Default: 8
  SPIKE_SRC_DIR       Spike source. Default: external/ibex/external/riscv-isa-sim
  SPIKE_PREFIX        Spike install prefix. Default: build/spike
  PK_SRC_DIR          pk source. Default: external/riscv-pk
  PK_PREFIX           pk install prefix. Default: build/pk
  IBEX_PKG_CONFIG_PATH
                      Ibex cosim pkg-config search path. Default: build/spike/lib/pkgconfig
  IBEX_SPIKE_COMMIT_LOG
                      Ibex Spike commit log. Default: log/ibex_spike_commit.log
  IBEX_COSIM_LOG      Ibex cosim compare log. Default: log/ibex_cosim_result.log
  IBEX_VERILATOR_VERSION
                      Expected Ibex Verilator version. Default: 5.010
                      Set to "any" to skip the exact-version guard.
  VEER_EL2_DIR        VeeR EL2 source tree. Default: external/Cores-VeeR-EL2
  VEER_EL2_MAX_CYCLES VeeR EL2 timeout cycle count. Default: 2000000
  VEER_EL2_COSIM_LOG  VeeR EL2 cosim compare log. Default: log/veer_el2_cosim_result.log
  VEER_EL2_SPIKE_COMMIT_LOG
                      VeeR EL2 Spike commit log. Default: log/veer_el2_spike_commit.log
  VEER_EL2_SPIKE_DTB  Prebuilt DTB used by Spike for VeeR cosim.
                      Default: build/src/top_cpu/veer_el2/spike_veer_el2.dtb

Tool overrides:
  TOOLCHAIN_PREFIX, PYTHON, CXX, IVERILOG, VVP, FUSESOC, PKG_CONFIG
EOF
}

require_command() {
	local name="$1"
	if command -v "$name" >/dev/null 2>&1; then
		return 0
	fi

	if [[ "$name" == "verilator" ]]; then
		local ver="${IBEX_VERILATOR_VERSION:-5.010}"
		if [[ "$ver" == "any" || -z "$ver" ]]; then
			ver="5.010"
		fi
		if command -v module >/dev/null 2>&1; then
			# Keep module output quiet unless the load fails.
			module load "openEDA/verilator/v$ver" >/dev/null 2>&1 || true
		fi
	fi

	if [[ "$name" == *"riscv32-unknown-elf-"* ]]; then
		if command -v module >/dev/null 2>&1; then
			module load riscv-toolchain/master-v20251230 >/dev/null 2>&1 || true
		fi
	fi

	if ! command -v "$name" >/dev/null 2>&1; then
		printf 'error: required command not found: %s\n' "$name" >&2
		return 1
	fi
}

require_file() {
	local path="$1"
	if [[ ! -f "$path" ]]; then
		printf 'error: required file not found: %s\n' "$path" >&2
		return 1
	fi
}

require_dir() {
	local path="$1"
	if [[ ! -d "$path" ]]; then
		printf 'error: required directory not found: %s\n' "$path" >&2
		return 1
	fi
}

is_target() {
	case "$1" in
		picorv32|ibex|veer_el2|all)
			return 0
			;;
		*)
			return 1
			;;
	esac
}

case_list() {
	local selector="$1"
	if [[ "$selector" == "all" ]]; then
		printf '%s\n' $FIRMWARE_CASES
	else
		printf '%s\n' "$selector"
	fi
}

target_list() {
	local selector="$1"
	if [[ "$selector" == "all" ]]; then
		printf '%s\n' picorv32 ibex veer_el2
	else
		printf '%s\n' "$selector"
	fi
}

case_dir() {
	printf '%s/%s\n' "$FIRMWARE_DIR" "$1"
}

case_obj_dir() {
	printf '%s/%s/obj\n' "$FIRMWARE_BUILD_DIR" "$1"
}

case_elf_path() {
	printf '%s/firmware.elf\n' "$(case_obj_dir "$1")"
}

case_hex_path() {
	printf '%s/firmware.hex\n' "$(case_obj_dir "$1")"
}

ibex_sim_dir() {
	printf '%s/local_spike_cosim_tb_ibex_cosim_0/sim-verilator\n' "$IBEX_BUILD_ROOT"
}

ibex_sim_exe() {
	printf '%s/Vtb_ibex\n' "$(ibex_sim_dir)"
}

case_veer_hex_path() {
	printf '%s/firmware_veer.hex\n' "$(case_obj_dir "$1")"
}

parse_uint32() {
	local value="$1"
	printf '%u\n' "$((value & 0xFFFFFFFF))"
}

require_ibex_reset_vector() {
	local reset_vector boot_addr expected_boot_addr effective_reset
	reset_vector="$(parse_uint32 "$RESET_VECTOR")"
	boot_addr="$(parse_uint32 "$IBEX_BOOT_ADDR")"
	expected_boot_addr="$(((reset_vector - 0x80) & 0xFFFFFFFF))"
	effective_reset="$(((boot_addr & 0xFFFFFF00) | 0x80))"

	if (((reset_vector & 0xFF) != 0x80)); then
		printf 'error: Ibex reset vector must have low byte 0x80, got 0x%08x\n' "$reset_vector" >&2
		return 1
	fi

	if ((boot_addr != expected_boot_addr)); then
		printf 'error: IBEX_BOOT_ADDR must be RESET_VECTOR-0x80, got 0x%08x expected 0x%08x\n' \
			"$boot_addr" "$expected_boot_addr" >&2
		return 1
	fi

	if ((effective_reset != reset_vector)); then
		printf 'error: IBEX_BOOT_ADDR produces reset vector 0x%08x, expected 0x%08x\n' \
			"$effective_reset" "$reset_vector" >&2
		return 1
	fi
}

require_firmware_tools() {
	require_command "${TOOLCHAIN_PREFIX}gcc"
	require_command "${TOOLCHAIN_PREFIX}objdump"
	require_command "${TOOLCHAIN_PREFIX}objcopy"
	require_command "$PYTHON"
	require_file "$SCRIPTS_DIR/makehex.py"
}

require_firmware_case() {
	local case_name="$1"
	local src_dir
	src_dir="$(case_dir "$case_name")"
	require_file "$FIRMWARE_COMMON_DIR/start.S"
	require_file "$FIRMWARE_COMMON_DIR/sections.lds"
	require_dir "$src_dir"
	require_file "$src_dir/main.c"
}

require_ibex_verilator_version() {
	require_command verilator

	if [[ "$IBEX_VERILATOR_VERSION" == "any" ]]; then
		return 0
	fi

	local banner version
	banner="$(verilator --version)"
	version="${banner#Verilator }"
	version="${version%% *}"

	if [[ "$version" != "$IBEX_VERILATOR_VERSION" ]]; then
		printf 'error: Ibex cosim expects Verilator %s, found %s\n' "$IBEX_VERILATOR_VERSION" "$version" >&2
		printf '       load the expected module, for example: module load openEDA/verilator/v%s\n' "$IBEX_VERILATOR_VERSION" >&2
		printf '       or set IBEX_VERILATOR_VERSION=any to bypass this project guard\n' >&2
		return 1
	fi
}

safe_remove_tree() {
	local path="$1"
	if [[ -z "$path" || "$path" == "/" ]]; then
		printf 'error: refusing to remove unsafe path: %s\n' "$path" >&2
		return 1
	fi
	rm -rf "$path"
}

safe_remove_tree_if_exists() {
	local path="$1"
	if [[ -e "$path" ]]; then
		safe_remove_tree "$path"
	fi
}

remove_paths() {
	local path
	for path in "$@"; do
		safe_remove_tree_if_exists "$path"
	done
}

firmware() {
	local case_name="$1"
	local src_dir obj_dir
	src_dir="$(case_dir "$case_name")"
	obj_dir="$(case_obj_dir "$case_name")"
	printf ' @ TB_HOME = %s\n' "$TB_HOME"
	printf ' @ TEST_NAME = %s\n' "$case_name"
	printf ' @ TEST_SRC_DIR = %s\n' "$src_dir"
	printf ' @ TEST_OBJ_DIR = %s\n' "$obj_dir"
	printf ' @ TOOLCHAIN_PREFIX = %s\n' "$TOOLCHAIN_PREFIX"
	require_firmware_tools
	require_firmware_case "$case_name"
	mkdir -p "$obj_dir"

	firmware_generic "$src_dir" "$obj_dir"
}

firmware_generic() {
	local src_dir="$1"
	local obj_dir="$2"
	local sources=()
	local source obj

	while IFS= read -r source; do
		sources+=("$source")
	done < <(find "$FIRMWARE_COMMON_DIR" "$src_dir" -maxdepth 1 \( -name '*.S' -o -name '*.c' \) -print | sort)

	if ((${#sources[@]} == 0)); then
		printf 'error: no firmware sources found under %s\n' "$src_dir" >&2
		return 1
	fi

	for source in "${sources[@]}"; do
		obj="$obj_dir/$(basename "${source%.*}").o"
		case "$source" in
			*.S)
				"${TOOLCHAIN_PREFIX}gcc" -static -c -march=rv32imc -mabi=ilp32 \
					-o "$obj" "$source"
				;;
			*.c)
				"${TOOLCHAIN_PREFIX}gcc" -static -c -march=rv32imc -mabi=ilp32 -Os \
					-ffreestanding -nostdlib -std=c99 -Wall -Wextra \
					-I"$FIRMWARE_COMMON_DIR" -I"$src_dir" \
					-o "$obj" "$source"
				;;
		esac
	done

	local objects=()
	for source in "${sources[@]}"; do
		objects+=("$obj_dir/$(basename "${source%.*}").o")
	done

	"${TOOLCHAIN_PREFIX}gcc" -march=rv32imc -mabi=ilp32 -Os -ffreestanding -nostdlib \
		-Wl,--build-id=none,-Bstatic,-T,"$FIRMWARE_COMMON_DIR/sections.lds",--strip-debug \
		-I"$FIRMWARE_COMMON_DIR" -I"$src_dir" \
		-o "$obj_dir/firmware.elf" \
		"${objects[@]}" \
		-lgcc

	"${TOOLCHAIN_PREFIX}objdump" -D "$obj_dir/firmware.elf" > "$obj_dir/firmware.lst"
	"${TOOLCHAIN_PREFIX}objcopy" -O binary "$obj_dir/firmware.elf" "$obj_dir/firmware.bin"
	"${TOOLCHAIN_PREFIX}objcopy" -O verilog "$obj_dir/firmware.elf" "$obj_dir/firmware_veer.hex"
	"$PYTHON" "$SCRIPTS_DIR/makehex.py" "$obj_dir/firmware.bin" 32768 > "$obj_dir/firmware.hex"
}

spike_ready() {
	[[ -x "$SPIKE_PREFIX/bin/spike" ]]
}

spike_cosim_api_ready() {
	local processor_h="$SPIKE_PREFIX/include/riscv/processor.h"
	[[ -f "$processor_h" ]] &&
		grep -q "set_ibex_flags" "$processor_h" &&
		grep -q "set_debug_module_range" "$processor_h" &&
		grep -q "set_mhpm_counter_num" "$processor_h"
}

pk_ready() {
	[[ -x "$MY_PK_PATH" ]]
}

vpi_ready() {
	[[ -f "$VPI_MODULE_DIR/libspike.vpi" ]]
}

ibex_spike_pkg_config_ready() {
	[[ -f "$SPIKE_PREFIX/lib/pkgconfig/riscv-fdt.pc" ]]
}

spike_deps_ready() {
	spike_ready && spike_cosim_api_ready && pk_ready && vpi_ready && ibex_spike_pkg_config_ready
}

firmware_ready() {
	local case_name="$1"
	[[ -f "$(case_elf_path "$case_name")" && -f "$(case_hex_path "$case_name")" ]]
}

veer_firmware_ready() {
	local case_name="$1"
	firmware_ready "$case_name" && [[ -f "$(case_veer_hex_path "$case_name")" ]]
}

picorv32_top_ready() {
	[[ -f "$SIM_VVP" && -f "$PICORV32_TOP_STAMP" ]] &&
		[[ "$(cat "$PICORV32_TOP_STAMP")" == "$(picorv32_top_stamp)" ]]
}

picorv32_use_dpi() {
	[[ "${PICORV32_COSIM_IF,,}" == "dpi" ]]
}

check_picorv32_cosim_if() {
	local mode="${PICORV32_COSIM_IF,,}"
	case "$mode" in
		vpi)
			return 0
			;;
		dpi)
			printf 'error: PICORV32_COSIM_IF=dpi is not supported on the current iverilog/vvp flow.\n' >&2
			printf '       keep PICORV32_COSIM_IF=vpi for picorv32 runs.\n' >&2
			return 1
			;;
		*)
			printf 'error: unsupported PICORV32_COSIM_IF=%s (expected: vpi or dpi)\n' "$PICORV32_COSIM_IF" >&2
			return 1
			;;
	esac
}

picorv32_top_stamp() {
	printf 'RESET_VECTOR=%s\nPICORV32_COSIM_IF=%s\n' \
		"$(printf '0x%08x' "$(parse_uint32 "$RESET_VECTOR")")" \
		"${PICORV32_COSIM_IF,,}"
}

ibex_top_ready() {
	[[ -x "$(ibex_sim_exe)" ]]
}

veer_el2_top_ready() {
	local expected
	expected="$(veer_el2_build_stamp)"
	[[ -x "$VEER_EL2_SIM_EXE" && -f "$VEER_EL2_BUILD_STAMP" ]] &&
		[[ "$(cat "$VEER_EL2_BUILD_STAMP")" == "$expected" ]]
}

veer_el2_build_stamp() {
	printf 'RESET_VECTOR=%s\nVEER_EL2_MAX_CYCLES=%s\nCOSIM=1\n' \
		"$(printf '0x%08x' "$(parse_uint32 "$RESET_VECTOR")")" \
		"$VEER_EL2_MAX_CYCLES"
	cksum \
		"$SRC_TOP_DIR/tb_veer_el2.sv" \
		"$SRC_TOP_DIR/tb_veer_el2_cosim.cc" \
		"$COSIM_SRC_DIR/cosim_session.cc" \
		"$COSIM_SRC_DIR/elf_utils.cc" \
		"$COSIM_SRC_DIR/spike_simulator.cc"
}

install_riscv_fdt_pc() {
	local pc_dir="$SPIKE_PREFIX/lib/pkgconfig"
	local pc_file="$pc_dir/riscv-fdt.pc"
	mkdir -p "$pc_dir"
	if [[ -f "$pc_file" ]]; then
		return 0
	fi
	cat > "$pc_file" <<EOF
prefix=$SPIKE_PREFIX
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include

Name: riscv-fdt
Description: RISC-V Flat Device Tree Manipulation
Version: git
Libs: -Wl,-rpath,\${libdir} -L\${libdir} -lfdt
Cflags: -I\${includedir}
EOF
}

spike() {
	require_file "$SPIKE_SRC_DIR/configure"
	if ! grep -q "set_ibex_flags" "$SPIKE_SRC_DIR/riscv/processor.h"; then
		printf 'error: Spike source is missing Ibex cosim APIs: %s\n' "$SPIKE_SRC_DIR" >&2
		printf '       use the lowRISC/Ibex-compatible Spike source, or set SPIKE_SRC_DIR explicitly.\n' >&2
		return 1
	fi
	if spike_ready && ! spike_cosim_api_ready; then
		printf 'Installed Spike is not Ibex-cosim compatible; rebuilding %s\n' "$SPIKE_PREFIX"
		safe_remove_tree_if_exists "$SPIKE_BUILD_DIR"
		safe_remove_tree_if_exists "$SPIKE_PREFIX"
	fi
	mkdir -p "$SPIKE_BUILD_DIR"
	(
		cd "$SPIKE_BUILD_DIR"
		"$SPIKE_SRC_DIR/configure" --enable-commitlog --enable-misaligned --prefix="$SPIKE_PREFIX"
		make -j"$BUILD_JOBS"
		make install
	)
	install_riscv_fdt_pc
}

pk() {
	require_file "$PK_SRC_DIR/configure"
	require_command "${PK_HOST}-gcc"
	require_command "${PK_HOST}-g++"
	require_command "${PK_HOST}-ar"
	require_command "${PK_HOST}-ranlib"
	require_command "${PK_HOST}-objcopy"
	require_command "${PK_HOST}-readelf"
	safe_remove_tree_if_exists "$PK_BUILD_DIR"
	mkdir -p "$PK_BUILD_DIR"
	(
		cd "$PK_BUILD_DIR"
		export CC="${PK_HOST}-gcc"
		export CXX="${PK_HOST}-g++"
		export AR="${PK_HOST}-ar"
		export RANLIB="${PK_HOST}-ranlib"
		export OBJCOPY="${PK_HOST}-objcopy"
		export READELF="${PK_HOST}-readelf"
		"$PK_SRC_DIR/configure" \
			--prefix="$PK_PREFIX" \
			--host="$PK_HOST" \
			--with-arch="$PK_ARCH"
		make -j"$BUILD_JOBS"
		make install
	)
}

deps() {
	spike
	pk
}

vpi() {
	require_command "$PKG_CONFIG"
	mkdir -p "$VPI_MODULE_DIR"
	local include_args=()
	local dir
	local cosim_sources=(
		"$COSIM_SRC_DIR/spike_dpi.cc"
		"$COSIM_SRC_DIR/cosim_bridge.cc"
		"$COSIM_SRC_DIR/cosim_session.cc"
		"$COSIM_SRC_DIR/cosim_config_policy.cc"
		"$COSIM_SRC_DIR/simulator_factory.cc"
		"$COSIM_SRC_DIR/elf_utils.cc"
		"$COSIM_SRC_DIR/spike_simulator.cc"
	)
	include_args+=("-I$COSIM_SRC_DIR")
	for dir in "${SPIKE_INCLUDE_DIRS[@]}" "${VPI_INCLUDE_DIRS[@]}"; do
		[[ -n "$dir" ]] && include_args+=("-I$dir")
	done

	local spike_pc_libs
	spike_pc_libs="$(
		PKG_CONFIG_PATH="$SPIKE_PREFIX/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}" \
			"$PKG_CONFIG" --libs riscv-riscv riscv-disasm riscv-fdt riscv-fesvr
	)"
	read -r -a spike_lib_args <<< "$spike_pc_libs"

	"$CXX" "${cosim_sources[@]}" -o "$VPI_MODULE_DIR/libspike.so" \
		-fPIC -shared -std=c++20 -include sys/syscall.h \
		"${include_args[@]}" \
		-L"$SPIKE_LIB_DIR" \
		-Wl,--start-group "${spike_lib_args[@]}" -Wl,--end-group \
		-lboost_regex -lboost_system -lpthread -lgmp -lmpfr -lmpc -ldl \
		-Wl,-rpath="$SPIKE_LIB_DIR" \
		-DVPI_WRAPPER
	cp "$VPI_MODULE_DIR/libspike.so" "$VPI_MODULE_DIR/libspike.vpi"

	"$CXX" "${cosim_sources[@]}" -o "$VPI_MODULE_DIR/libspike_dpi.so" \
		-fPIC -shared -std=c++20 -include sys/syscall.h \
		"${include_args[@]}" \
		-L"$SPIKE_LIB_DIR" \
		-Wl,--start-group "${spike_lib_args[@]}" -Wl,--end-group \
		-lboost_regex -lboost_system -lpthread -lgmp -lmpfr -lmpc -ldl \
		-Wl,-rpath="$SPIKE_LIB_DIR" \
		-DDPI_WRAPPER
}

build_spike_deps() {
	if spike_ready; then
		install_riscv_fdt_pc
	fi
	if spike_deps_ready; then
		printf 'Spike dependencies are up to date under %s\n' "$BUILD_DIR"
		return 0
	fi
	if ! spike_ready || ! spike_cosim_api_ready; then
		spike
	else
		install_riscv_fdt_pc
	fi
	if ! pk_ready; then
		pk
	fi
	vpi
}

ensure_spike_deps() {
	if ! spike_deps_ready; then
		build_spike_deps
	fi
}

build_firmware_case() {
	local case_name="$1"
	ensure_spike_deps
	firmware "$case_name"
}

ensure_firmware_case() {
	local case_name="$1"
	if ! firmware_ready "$case_name"; then
		build_firmware_case "$case_name"
	fi
}

ensure_veer_firmware_case() {
	local case_name="$1"
	if ! veer_firmware_ready "$case_name"; then
		build_firmware_case "$case_name"
	fi
}

build_picorv32_top() {
	check_picorv32_cosim_if
	ensure_spike_deps
	require_file "$VPI_MODULE_DIR/libspike.vpi"
	require_file "$PICORV32_RTL"
	mkdir -p "$TOP_BUILD_DIR"

	"$IVERILOG" -g2012 -o "$SIM_VVP" "$SRC_TOP_DIR/tb_picorv32.v" "$PICORV32_RTL" \
		-DVPI_WRAPPER -DCOMPRESSED_ISA -DRISCV_FORMAL \
		-P"tb_picorv32.RESET_VECTOR=$(parse_uint32 "$RESET_VECTOR")"
	picorv32_top_stamp > "$PICORV32_TOP_STAMP"
}

ensure_picorv32_top() {
	ensure_spike_deps
	if ! picorv32_top_ready; then
		build_picorv32_top
	fi
}

sim_picorv32() {
	check_picorv32_cosim_if
	local elf_path="$1"
	local hex_path="$2"
	require_file "$elf_path"
	require_file "$hex_path"
	require_file "$VPI_MODULE_DIR/libspike.vpi"
	require_file "$SIM_VVP"
	mkdir -p "$DUMP_DIR" "$(dirname "$PICORV32_COSIM_LOG")" "$(dirname "$PICORV32_SPIKE_COMMIT_LOG")"
	LD_LIBRARY_PATH="$SPIKE_LIB_DIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
		PICORV32_COSIM_LOG="$PICORV32_COSIM_LOG" \
		PICORV32_SPIKE_COMMIT_LOG="$PICORV32_SPIKE_COMMIT_LOG" \
		MY_PK_PATH="$MY_PK_PATH" "$VVP" -M "$VPI_MODULE_DIR" -m libspike "$SIM_VVP" +trace \
		"+ELF_PATH=$elf_path" \
		"+firmware=$hex_path" \
		"+PK_PATH=$MY_PK_PATH" \
		"+ISA=$MY_ISA"
}

check_ibex_pkg_config() {
	require_command "$PKG_CONFIG"

	if ! PKG_CONFIG_PATH="$IBEX_PKG_CONFIG_PATH" "$PKG_CONFIG" --exists riscv-riscv riscv-disasm riscv-fdt; then
		printf 'error: missing Ibex cosim Spike pkg-config packages: riscv-riscv riscv-disasm riscv-fdt\n' >&2
		printf '       set IBEX_PKG_CONFIG_PATH to an Ibex-compatible Spike install\n' >&2
		return 1
	fi
}

check_ibex_cosim_build_deps() {
	require_command "$FUSESOC"
	require_ibex_verilator_version
	require_ibex_reset_vector
	require_file "$SRC_TOP_DIR/tb_ibex_cosim.core"
	require_file "$IBEX_DIR/examples/simple_system/ibex_simple_system.core"
	check_ibex_pkg_config
}

check_ibex_cosim_run_deps() {
	require_ibex_reset_vector
	check_ibex_pkg_config
}

build_ibex_cosim() {
	ensure_spike_deps
	check_ibex_cosim_build_deps
	mkdir -p "$IBEX_BUILD_ROOT" "$DUMP_DIR"
	(
		cd "$IBEX_DIR"
		MAKEFLAGS="-j$BUILD_JOBS" \
		PKG_CONFIG_PATH="$IBEX_PKG_CONFIG_PATH" "$FUSESOC" \
			--cores-root=. \
			--cores-root="$TB_HOME/src/top_cpu" \
			run \
			--target=sim \
			--setup \
			--build \
			--build-root "$IBEX_BUILD_ROOT" \
			"$IBEX_FUSESOC_CORE" \
			--RV32E=0 \
			--RV32M=ibex_pkg::RV32MFast \
			--RamBaseAddr="$IBEX_RAM_BASE" \
			--BootAddr="$(parse_uint32 "$IBEX_BOOT_ADDR")"
	)
}

sim_ibex() {
	local elf_path="$1"
	local sim_exe
	sim_exe="$(ibex_sim_exe)"
	check_ibex_cosim_run_deps
	require_file "$sim_exe"
	require_file "$elf_path"
	mkdir -p "$DUMP_DIR" "$LOG_DIR" \
		"$(dirname "$IBEX_TRACE_FILE_BASE")" \
		"$(dirname "$IBEX_SIMPLE_SYSTEM_LOG")" \
		"$(dirname "$IBEX_PCOUNT_CSV")" \
		"$(dirname "$IBEX_COSIM_LOG")" \
		"$(dirname "$IBEX_SPIKE_COMMIT_LOG")"
	(
		cd "$TB_HOME"
		IBEX_RESET_VECTOR="$RESET_VECTOR" \
		IBEX_RAM_BASE="$IBEX_RAM_BASE" \
		PKG_CONFIG_PATH="$IBEX_PKG_CONFIG_PATH" \
		TEST_ELF="$elf_path" \
		SPIKE_COMMIT_LOG="$IBEX_SPIKE_COMMIT_LOG" \
		IBEX_COSIM_LOG="$IBEX_COSIM_LOG" \
		IBEX_PCOUNT_CSV="$IBEX_PCOUNT_CSV" \
			"$sim_exe" --meminit=ram,"$elf_path" \
			"+ibex_tracer_file_base=$IBEX_TRACE_FILE_BASE" \
			"+ibex_simple_system_log=$IBEX_SIMPLE_SYSTEM_LOG"
	)
}

veer_el2_verilator_warning_flags() {
	local no_implicit="-Wno-IMPLICITSTATIC"
	local version_raw version_num
	version_raw="$(verilator --version 2>/dev/null | awk '{print $2}')"
	version_num="${version_raw//./}"

	if [[ -n "$version_num" && "$version_num" =~ ^[0-9]+$ && "$version_num" -lt 5006 ]]; then
		no_implicit="-Wno-IMPLICIT"
	fi

	printf '%s\n' "$no_implicit -Wno-TIMESCALEMOD -Wno-ASCRANGE -Wno-CASEINCOMPLETE -Wno-INITIALDLY -Wno-WIDTH -Wno-UNOPTFLAT -Wno-LATCH"
}

check_veer_el2_build_deps() {
	require_command verilator
	require_command make
	require_command "$PKG_CONFIG"
	require_file "$VEER_EL2_DIR/configs/veer.config"
	require_file "$VEER_EL2_DIR/testbench/tb_top.sv"
	require_file "$VEER_EL2_DIR/testbench/flist"
	require_file "$SRC_TOP_DIR/tb_veer_el2.sv"
	require_file "$SRC_TOP_DIR/tb_veer_el2_cosim.cc"
	PKG_CONFIG_PATH="$SPIKE_PREFIX/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}" \
		"$PKG_CONFIG" --exists riscv-riscv riscv-disasm riscv-fdt riscv-fesvr
}

generate_veer_el2_config() {
	local reset_vector
	reset_vector="$(printf '0x%08x' "$(parse_uint32 "$RESET_VECTOR")")"
	mkdir -p "$VEER_EL2_CONFIG_DIR"
	RV_ROOT="$VEER_EL2_DIR" BUILD_PATH="$VEER_EL2_CONFIG_DIR" "$VEER_EL2_DIR/configs/veer.config" \
		-target=default \
		-set build_axi4 \
		-set=reset_vec="$reset_vector" \
		-set=icache_waypack=0 \
		-set bitmanip_zba \
		-set bitmanip_zbb \
		-set bitmanip_zbc \
		-set bitmanip_zbe \
		-set bitmanip_zbf \
		-set bitmanip_zbp \
		-set bitmanip_zbr \
		-set bitmanip_zbs \
		-set=fpga_optimize=0
}

generate_veer_el2_flist() {
	local source_flist="$VEER_EL2_DIR/testbench/flist"
	local line
	mkdir -p "$VEER_EL2_BUILD_ROOT"
	: > "$VEER_EL2_FLIST"
	while IFS= read -r line; do
		printf '%s\n' "${line//\$RV_ROOT/$VEER_EL2_DIR}" >> "$VEER_EL2_FLIST"
	done < "$source_flist"
}

generate_veer_el2_spike_dtb() {
	require_command dtc
	mkdir -p "$(dirname "$VEER_EL2_SPIKE_DTS")" "$(dirname "$VEER_EL2_SPIKE_DTB")"

	local ram_base ram_size
	ram_base="$(parse_uint32 "$VEER_EL2_RAM_BASE")"
	ram_size="$(parse_uint32 "$VEER_EL2_RAM_SIZE")"

	{
		printf '/dts-v1/;\n\n'
		printf '/ {\n'
		printf '  #address-cells = <2>;\n'
		printf '  #size-cells = <2>;\n'
		printf '  compatible = "ucbbar,spike-bare-dev";\n'
		printf '  model = "ucbbar,spike-bare";\n'
		printf '  chosen { bootargs = "console=hvc0 earlycon=sbi"; };\n'
		printf '  cpus {\n'
		printf '    #address-cells = <1>;\n'
		printf '    #size-cells = <0>;\n'
		printf '    timebase-frequency = <1000000>;\n'
		printf '    CPU0: cpu@0 {\n'
		printf '      device_type = "cpu";\n'
		printf '      reg = <0>;\n'
		printf '      status = "okay";\n'
		printf '      compatible = "riscv";\n'
		printf '      riscv,isa = "rv32imc";\n'
		printf '      mmu-type = "riscv,sv32";\n'
		printf '      riscv,pmpregions = <16>;\n'
		printf '      riscv,pmpgranularity = <4>;\n'
		printf '      clock-frequency = <1000000000>;\n'
		printf '      ibex,secure-ibex = <0>;\n'
		printf '      ibex,icache-en = <0>;\n'
		printf '      CPU0_intc: interrupt-controller {\n'
		printf '        #address-cells = <2>;\n'
		printf '        #interrupt-cells = <1>;\n'
		printf '        interrupt-controller;\n'
		printf '        compatible = "riscv,cpu-intc";\n'
		printf '      };\n'
		printf '    };\n'
		printf '  };\n'
		printf '  memory@%08x {\n' "$ram_base"
		printf '    device_type = "memory";\n'
		printf '    reg = <0x0 0x%08x 0x0 0x%08x>;\n' "$ram_base" "$ram_size"
		printf '  };\n'
		printf '  soc {\n'
		printf '    #address-cells = <2>;\n'
		printf '    #size-cells = <2>;\n'
		printf '    compatible = "ucbbar,spike-bare-soc", "simple-bus";\n'
		printf '    ranges;\n'
		printf '    clint@2000000 {\n'
		printf '      compatible = "riscv,clint0";\n'
		printf '      interrupts-extended = <&CPU0_intc 3 &CPU0_intc 7>;\n'
		printf '      reg = <0x0 0x02000000 0x0 0x000c0000>;\n'
		printf '    };\n'
		printf '  };\n'
		printf '  htif { compatible = "ucb,htif0"; };\n'
		printf '};\n'
	} > "$VEER_EL2_SPIKE_DTS"

	dtc -O dtb -o "$VEER_EL2_SPIKE_DTB" "$VEER_EL2_SPIKE_DTS"
}

build_veer_el2_top() {
	ensure_spike_deps
	check_veer_el2_build_deps
	generate_veer_el2_config
	generate_veer_el2_flist

	local warning_flags
	warning_flags="$(veer_el2_verilator_warning_flags)"
	local spike_pc_cflags spike_pc_libs
	spike_pc_cflags="$(
		PKG_CONFIG_PATH="$SPIKE_PREFIX/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}" \
			"$PKG_CONFIG" --cflags riscv-riscv riscv-disasm riscv-fdt riscv-fesvr
	)"
	spike_pc_libs="$(
		PKG_CONFIG_PATH="$SPIKE_PREFIX/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}" \
			"$PKG_CONFIG" --libs riscv-riscv riscv-disasm riscv-fdt riscv-fesvr
	)"
	mkdir -p "$VEER_EL2_BUILD_ROOT" "$DUMP_DIR" "$LOG_DIR"

	(
		cd "$VEER_EL2_BUILD_ROOT"
		export RV_ROOT="$VEER_EL2_DIR"
		verilator --cc \
			-CFLAGS "-std=c++20 -include sys/syscall.h -I$COSIM_SRC_DIR -I$SRC_TOP_DIR $spike_pc_cflags -I$SPIKE_ROOT/include/riscv -I$SPIKE_ROOT/include/fesvr" \
			-LDFLAGS "-Wl,--start-group $spike_pc_libs -Wl,--end-group -lboost_regex -lboost_system -lpthread -lgmp -lmpfr -lmpc -ldl" \
			+define+RV_OPENSOURCE \
			-I"$SRC_TOP_DIR" \
			-I"$VEER_EL2_CONFIG_DIR" \
			-I"$VEER_EL2_DIR/testbench/axi4_mux" \
			-I"$VEER_EL2_DIR/testbench" \
			"$VEER_EL2_CONFIG_DIR/common_defines.vh" \
			"$VEER_EL2_DIR/design/include/el2_def.sv" \
			"$VEER_EL2_CONFIG_DIR/el2_pdef.vh" \
			-f "$VEER_EL2_FLIST" \
			"$VEER_EL2_DIR/testbench/tb_top_pkg.sv" \
			"$VEER_EL2_DIR/testbench/tb_top.sv" \
			"$VEER_EL2_DIR/testbench/ahb_sif.sv" \
			"$VEER_EL2_DIR/testbench/ahb_lite_2to1_mux.sv" \
			"$VEER_EL2_DIR/testbench/ahb_lsu_dma_bridge.sv" \
			"$VEER_EL2_DIR/testbench/axi4_mux/axi_crossbar_wrap_2x1.v" \
			"$VEER_EL2_DIR/testbench/axi4_mux/arbiter.v" \
			"$VEER_EL2_DIR/testbench/axi4_mux/axi_crossbar_addr.v" \
			"$VEER_EL2_DIR/testbench/axi4_mux/axi_crossbar_rd.v" \
			"$VEER_EL2_DIR/testbench/axi4_mux/axi_crossbar.v" \
			"$VEER_EL2_DIR/testbench/axi4_mux/axi_crossbar_wr.v" \
			"$VEER_EL2_DIR/testbench/axi4_mux/axi_register_rd.v" \
			"$VEER_EL2_DIR/testbench/axi4_mux/axi_register_wr.v" \
			"$VEER_EL2_DIR/testbench/axi4_mux/priority_encoder.v" \
			"$SRC_TOP_DIR/tb_veer_el2.sv" \
			"$SRC_TOP_DIR/tb_veer_el2_cosim.cc" \
			"$SRC_TOP_DIR/cosim_top_utils.cc" \
			"$COSIM_SRC_DIR/cosim_bridge.cc" \
			"$COSIM_SRC_DIR/cosim_session.cc" \
		"$COSIM_SRC_DIR/cosim_config_policy.cc" \
		"$COSIM_SRC_DIR/simulator_factory.cc" \
		"$COSIM_SRC_DIR/elf_utils.cc" \
		"$COSIM_SRC_DIR/spike_simulator.cc" \
			--top-module tb_top \
			--exe \
			--autoflush \
			--timing \
			-GMAX_CYCLES="$VEER_EL2_MAX_CYCLES" \
			$warning_flags \
			-fno-table
		make -j"$BUILD_JOBS" -C "$VEER_EL2_OBJ_DIR" -f Vtb_top.mk OPT_FAST="-Os"
	)
	veer_el2_build_stamp > "$VEER_EL2_BUILD_STAMP"
}

sim_veer_el2() {
	local elf_path="$1"
	local veer_hex_path="$2"
	require_file "$elf_path"
	require_file "$veer_hex_path"
	require_file "$VEER_EL2_SIM_EXE"
	mkdir -p "$DUMP_DIR" "$LOG_DIR" \
		"$(dirname "$VEER_EL2_COSIM_LOG")" \
		"$(dirname "$VEER_EL2_SPIKE_COMMIT_LOG")"
	generate_veer_el2_spike_dtb
	(
		cd "$VEER_EL2_BUILD_ROOT"
		cp "$veer_hex_path" program.hex
		LD_LIBRARY_PATH="$SPIKE_LIB_DIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
		MY_ISA="$MY_ISA" \
		VEER_EL2_RAM_BASE="$VEER_EL2_RAM_BASE" \
		VEER_EL2_RAM_SIZE="$VEER_EL2_RAM_SIZE" \
		VEER_EL2_MAX_CYCLES="$VEER_EL2_MAX_CYCLES" \
		VEER_EL2_COSIM_LOG="$VEER_EL2_COSIM_LOG" \
		VEER_EL2_SPIKE_COMMIT_LOG="$VEER_EL2_SPIKE_COMMIT_LOG" \
		VEER_EL2_SPIKE_DTB="$VEER_EL2_SPIKE_DTB" \
			"$VEER_EL2_SIM_EXE" "+ELF_PATH=$elf_path"
	)
}

build_top() {
	local target="$1"
	case "$target" in
		picorv32)
			build_picorv32_top
			;;
		ibex)
			build_ibex_cosim
			;;
		veer_el2)
			build_veer_el2_top
			;;
		all)
			build_top picorv32
			build_top ibex
			build_top veer_el2
			;;
		*)
			printf 'error: unknown build target: %s\n\n' "$target" >&2
			usage >&2
			exit 2
			;;
	esac
}

build_firmware() {
	local selector="${1:-all}"
	local case_name
	while IFS= read -r case_name; do
		build_firmware_case "$case_name"
	done < <(case_list "$selector")
}

build_command() {
	local kind="${1:-}"
	case "$kind" in
		spike)
			if (($# != 1)); then
				printf 'error: build spike takes no extra arguments\n\n' >&2
				usage >&2
				exit 2
			fi
			build_spike_deps
			;;
		-f)
			shift
			build_firmware "${1:-all}"
			if (($# > 1)); then
				printf 'error: build -f accepts at most one case selector\n\n' >&2
				usage >&2
				exit 2
			fi
			;;
		-t)
			shift
			build_top "${1:-all}"
			if (($# > 1)); then
				printf 'error: build -t accepts at most one target selector\n\n' >&2
				usage >&2
				exit 2
			fi
			;;
		*)
			printf 'error: unknown build selector: %s\n\n' "${kind:-<missing>}" >&2
			usage >&2
			exit 2
			;;
	esac
}

run_one() {
	local target="$1"
	local case_name="$2"
	local elf_path hex_path veer_hex_path
	elf_path="$(case_elf_path "$case_name")"
	hex_path="$(case_hex_path "$case_name")"
	veer_hex_path="$(case_veer_hex_path "$case_name")"
	case "$target" in
		picorv32)
			ensure_firmware_case "$case_name"
			ensure_picorv32_top
			sim_picorv32 "$elf_path" "$hex_path"
			;;
		ibex)
			ensure_firmware_case "$case_name"
			if ! ibex_top_ready; then
				build_ibex_cosim
			fi
			sim_ibex "$elf_path"
			;;
		veer_el2)
			ensure_veer_firmware_case "$case_name"
			if ! veer_el2_top_ready; then
				build_veer_el2_top
			fi
			sim_veer_el2 "$elf_path" "$veer_hex_path"
			;;
		*)
			printf 'error: unknown run target: %s\n\n' "$target" >&2
			usage >&2
			exit 2
			;;
	esac
}

target_cosim_log_path() {
	local target="$1"
	case "$target" in
		picorv32) printf '%s\n' "$PICORV32_COSIM_LOG" ;;
		ibex) printf '%s\n' "$IBEX_COSIM_LOG" ;;
		veer_el2) printf '%s\n' "$VEER_EL2_COSIM_LOG" ;;
		*) return 1 ;;
	esac
}

detect_cosim_status() {
	local target="$1"
	local cosim_log
	cosim_log="$(target_cosim_log_path "$target" || true)"
	if [[ -z "$cosim_log" || ! -f "$cosim_log" ]]; then
		printf '%s\n' "NA"
		return 0
	fi
	if rg -qi "mismatch|cosim fail|co-simulation failed" "$cosim_log"; then
		printf '%s\n' "FAIL"
	else
		printf '%s\n' "PASS"
	fi
}

detect_case_pass_status() {
	local case_name="$1"
	local run_log="$2"
	if rg -q "(^|[^A-Z])PASS([^A-Z]|$)" "$run_log"; then
		printf '%s\n' "PASS"
		return 0
	fi
	if rg -q "OUT:[[:space:]]+123456789" "$run_log"; then
		printf '%s\n' "PASS"
		return 0
	fi
	printf '%s\n' "FAIL"
}

run_command() {
	local target="${1:-all}"
	local case_selector="${2:-$TEST_NAME}"
	local target_name case_name
	local run_log sim_exit pass_status cosim_status result
	local -a rows=()
	local fail_count=0

	if (($# > 2)); then
		printf 'error: run accepts at most target and case selectors\n\n' >&2
		usage >&2
		exit 2
	fi

	if ! is_target "$target"; then
		printf 'error: unknown run target: %s\n\n' "$target" >&2
		usage >&2
		exit 2
	fi

	while IFS= read -r target_name; do
		while IFS= read -r case_name; do
			run_log="$LOG_DIR/run_${target_name}_${case_name}.log"
			mkdir -p "$LOG_DIR"
			printf '\n[RUN] target=%s case=%s log=%s\n' "$target_name" "$case_name" "$run_log"
			if run_one "$target_name" "$case_name" 2>&1 | tee "$run_log"; then
				sim_exit=0
			else
				sim_exit=$?
			fi

			pass_status="$(detect_case_pass_status "$case_name" "$run_log")"

			cosim_status="$(detect_cosim_status "$target_name")"

			if [[ "$sim_exit" -eq 0 && "$pass_status" == "PASS" && "$cosim_status" != "FAIL" ]]; then
				result="PASS"
			else
				result="FAIL"
				fail_count=$((fail_count + 1))
			fi

			rows+=("$target_name|$case_name|$sim_exit|$pass_status|$cosim_status|$result|$run_log")
		done < <(case_list "$case_selector")
	done < <(target_list "$target")

	printf '\n'
	printf '%-10s %-12s %-8s %-12s %-10s %-8s %s\n' "target" "case" "exit" "pass_marker" "cosim" "result" "run_log"
	printf '%-10s %-12s %-8s %-12s %-10s %-8s %s\n' "------" "----" "----" "-----------" "-----" "------" "-------"
	local row
	for row in "${rows[@]}"; do
		IFS='|' read -r target_name case_name sim_exit pass_status cosim_status result run_log <<<"$row"
		printf '%-10s %-12s %-8s %-12s %-10s %-8s %s\n' \
			"$target_name" "$case_name" "$sim_exit" "$pass_status" "$cosim_status" "$result" "$run_log"
	done

	if ((fail_count > 0)); then
		printf '\nerror: %d run(s) failed\n' "$fail_count" >&2
		return 1
	fi
}

all() {
	run_command all all
}

clean() {
	remove_paths \
		"$FIRMWARE_BUILD_DIR" \
		"$SRC_BUILD_DIR" \
		"$DUMP_DIR" \
		"$LOG_DIR" \
		"$TB_HOME/trace_core_00000000.log" \
		"$TB_HOME/tb_ibex_pcount.csv" \
		"$TB_HOME/ibex_simple_system.log"
}

clean_all() {
	remove_paths \
		"$BUILD_DIR" \
		"$DUMP_DIR" \
		"$LOG_DIR" \
		"$TB_HOME/trace_core_00000000.log" \
		"$TB_HOME/tb_ibex_pcount.csv" \
		"$TB_HOME/ibex_simple_system.log"
}

command="${1:-help}"
case "$command" in
	help|-h|--help)
		usage
		;;
	build)
		shift
		build_command "$@"
		;;
	run)
		shift
		run_command "$@"
		;;
	all)
		all
		;;
	clean)
		clean
		;;
	clean-all)
		clean_all
		;;
	*)
		printf 'error: unknown command: %s\n\n' "$command" >&2
		usage >&2
		exit 2
		;;
esac
