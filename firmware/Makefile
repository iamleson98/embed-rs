# STM32F103 Bare-Metal Thermal Monitor — Makefile
# Requires: arm-none-eabi-gcc toolchain

# ---- Toolchain ----
PREFIX  = arm-none-eabi-
CC      = $(PREFIX)gcc
AS      = $(PREFIX)as
LD      = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
SIZE    = $(PREFIX)size

# ---- Project ----
TARGET  = thermal_monitor
BUILD   = build

# ---- Sources ----
SRC_DIR = src
INC_DIR = include

SRCS    = $(wildcard $(SRC_DIR)/*.c)
OBJS    = $(patsubst $(SRC_DIR)/%.c, $(BUILD)/%.o, $(SRCS))

# ---- Flags ----
MCU     = -mcpu=cortex-m3 -mthumb

CFLAGS  = $(MCU) \
          -Wall -Wextra -Werror \
          -std=c11 \
          -Os -g3 \
          -ffunction-sections -fdata-sections \
          -fno-common \
          -I$(INC_DIR)

LDFLAGS = $(MCU) \
          -T linker.ld \
          -Wl,--gc-sections \
          -Wl,-Map=$(BUILD)/$(TARGET).map \
          --specs=nosys.specs \
          -nostartfiles \
          -lc -lnosys

# ---- Targets ----
.PHONY: all clean flash size disasm

all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).bin $(BUILD)/$(TARGET).hex
	@echo "Build complete."
	@$(SIZE) $(BUILD)/$(TARGET).elf

$(BUILD)/%.o: $(SRC_DIR)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/$(TARGET).elf: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD)/$(TARGET).hex: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD):
	mkdir -p $(BUILD)

size: $(BUILD)/$(TARGET).elf
	$(SIZE) --format=berkeley $<

disasm: $(BUILD)/$(TARGET).elf
	$(OBJDUMP) -d -S $< > $(BUILD)/$(TARGET).lst

clean:
	rm -rf $(BUILD)

# ---- Flash via OpenOCD (ST-Link) ----
flash: $(BUILD)/$(TARGET).bin
	openocd -f interface/stlink.cfg \
	        -f target/stm32f1x.cfg \
	        -c "program $(BUILD)/$(TARGET).bin 0x08000000 verify reset exit"
