TOOLCHAIN_PREFIX ?= riscv32-unknown-elf-
CC := $(TOOLCHAIN_PREFIX)gcc
OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy
OBJDUMP := $(TOOLCHAIN_PREFIX)objdump

CASE_NAME ?= $(notdir $(CURDIR))
TARGET ?= firmware
CASE_BUILD_DIR ?= $(if $(FW_BUILD_DIR),$(FW_BUILD_DIR),build)
INSTALL_DIR ?= ../../build/firmware/$(CASE_NAME)/obj
COMMON_DIR ?= ../common
SCRIPTS_DIR ?= ../../scripts
PYTHON ?= python3
ARCH ?= rv32imc
ABI ?= ilp32
HEX_WORDS ?= 32768

LINKER_SCRIPT ?= $(COMMON_DIR)/sections.lds

TARGET_ELF := $(CASE_BUILD_DIR)/$(TARGET).elf
TARGET_BIN := $(CASE_BUILD_DIR)/$(TARGET).bin
TARGET_HEX := $(CASE_BUILD_DIR)/$(TARGET).hex
TARGET_VEER_HEX := $(CASE_BUILD_DIR)/$(TARGET)_veer.hex
TARGET_LST := $(CASE_BUILD_DIR)/$(TARGET).lst
TARGET_MAP := $(CASE_BUILD_DIR)/$(TARGET).map

CASE_C_SRCS := $(wildcard *.c)
CASE_ASM_SRCS := $(wildcard *.S)
COMMON_C_SRCS := $(wildcard $(COMMON_DIR)/*.c)
COMMON_ASM_SRCS := $(wildcard $(COMMON_DIR)/*.S)

CASE_C_OBJS := $(patsubst %.c,$(CASE_BUILD_DIR)/case_%.o,$(notdir $(CASE_C_SRCS)))
CASE_ASM_OBJS := $(patsubst %.S,$(CASE_BUILD_DIR)/case_%.o,$(notdir $(CASE_ASM_SRCS)))
COMMON_C_OBJS := $(patsubst %.c,$(CASE_BUILD_DIR)/common_%.o,$(notdir $(COMMON_C_SRCS)))
COMMON_ASM_OBJS := $(patsubst %.S,$(CASE_BUILD_DIR)/common_%.o,$(notdir $(COMMON_ASM_SRCS)))
OBJS := $(COMMON_ASM_OBJS) $(CASE_ASM_OBJS) $(COMMON_C_OBJS) $(CASE_C_OBJS)

ELF2HEX_AUTO := $(firstword $(foreach tool,$(TOOLCHAIN_PREFIX)elf2hex riscv64-unknown-elf-elf2hex elf2hex,$(shell command -v $(tool) 2>/dev/null)))
ELF2HEX_BIN := $(or $(ELF2HEX),$(ELF2HEX_AUTO))

FW_CFLAGS := -static -c -march=$(ARCH) -mabi=$(ABI) -Os \
	-ffreestanding -nostdlib -std=c99 -Wall -Wextra \
	-I$(COMMON_DIR) -I.
FW_ASFLAGS := -static -c -march=$(ARCH) -mabi=$(ABI) -I$(COMMON_DIR) -I.
FW_LDFLAGS := -march=$(ARCH) -mabi=$(ABI) -Os -ffreestanding -nostdlib \
	-Wl,--build-id=none,-Bstatic,-T,$(LINKER_SCRIPT),-Map,$(TARGET_MAP),--strip-debug

INSTALL_ARTIFACTS := $(TARGET_ELF) $(TARGET_BIN) $(TARGET_HEX) $(TARGET_VEER_HEX) $(TARGET_LST) $(TARGET_MAP)

.DELETE_ON_ERROR:
.PHONY: all firmware install clean list

all: firmware

firmware: $(TARGET_ELF) $(TARGET_BIN) $(TARGET_HEX) $(TARGET_VEER_HEX) $(TARGET_LST)

install: firmware
	@mkdir -p $(INSTALL_DIR)
	cp -f $(INSTALL_ARTIFACTS) $(INSTALL_DIR)/

$(CASE_BUILD_DIR):
	@mkdir -p $@

$(CASE_BUILD_DIR)/case_%.o: %.c | $(CASE_BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(FW_CFLAGS) -o $@ $<

$(CASE_BUILD_DIR)/case_%.o: %.S | $(CASE_BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(FW_ASFLAGS) -o $@ $<

$(CASE_BUILD_DIR)/common_%.o: $(COMMON_DIR)/%.c | $(CASE_BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(FW_CFLAGS) -o $@ $<

$(CASE_BUILD_DIR)/common_%.o: $(COMMON_DIR)/%.S | $(CASE_BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(FW_ASFLAGS) -o $@ $<

$(TARGET_ELF): $(OBJS) $(LINKER_SCRIPT)
	@mkdir -p $(dir $@)
	$(CC) $(FW_LDFLAGS) -I$(COMMON_DIR) -I. -o $@ $(OBJS) -lgcc

$(TARGET_BIN): $(TARGET_ELF)
	@mkdir -p $(dir $@)
	$(OBJCOPY) -O binary $< $@

ifneq ($(strip $(ELF2HEX_BIN)),)
$(TARGET_HEX): $(TARGET_ELF)
	@mkdir -p $(dir $@)
	$(ELF2HEX_BIN) --bit-width 32 --input $< --output $@
else
$(TARGET_HEX): $(TARGET_BIN)
	@mkdir -p $(dir $@)
	$(PYTHON) $(SCRIPTS_DIR)/makehex.py $< $(HEX_WORDS) > $@
endif

$(TARGET_VEER_HEX): $(TARGET_ELF)
	@mkdir -p $(dir $@)
	$(OBJCOPY) -O verilog $< $@

$(TARGET_LST): $(TARGET_ELF)
	@mkdir -p $(dir $@)
	$(OBJDUMP) -D $< > $@

list:
	@ls -lh $(CASE_BUILD_DIR) 2>/dev/null || true

clean:
	rm -rf $(CASE_BUILD_DIR)
