# 项目编译目标名
TARGET = akaLink_Compact_Dfu
# 调试信息
DEBUG = 1
# 优化等级
OPT = -Os
# 链接时优化
LTO = -flto

# 检测操作系统
ifeq ($(OS),Windows_NT)
    DETECTED_OS := Windows_NT
else
    DETECTED_OS := $(shell uname -s)
endif
OS := $(DETECTED_OS)


# 编译临时文件目录
BUILD_DIR = build
EXEC_DIR = build_exec


# C源文件宏定义
C_DEFS += -DCONFIG_USB_HS

# C头文件目录
C_INCLUDES += -Iapp/
C_INCLUDES += -Idrv/
C_INCLUDES += -Iusb/
C_INCLUDES += -Isys/
C_INCLUDES += -Ivfat/

C_INCLUDES += -I../Drivers/inc

C_INCLUDES += -I../Middleware/CherryUSB/common
C_INCLUDES += -I../Middleware/CherryUSB/core
C_INCLUDES += -I../Middleware/CherryUSB/class/msc
C_INCLUDES += -I../Middleware/CherryUSB/port/ch32/ch32hs

C_INCLUDES += -I../Middleware/CherryRB

# C源文件
C_SOURCES += $(wildcard ../Drivers/src/*.c)

C_SOURCES += $(wildcard app/*.c)
C_SOURCES += $(wildcard drv/*.c)
C_SOURCES += $(wildcard usb/*.c)
C_SOURCES += $(wildcard vfat/*.c)

C_SOURCES += $(wildcard ../Middleware/CherryRB/*.c)
C_SOURCES += $(wildcard ../Middleware/CherryUSB/class/msc/*.c)
C_SOURCES += $(wildcard ../Middleware/CherryUSB/port/ch32/ch32hs/*.c)
C_SOURCES += $(wildcard ../Middleware/CherryUSB/core/*.c)


# 汇编文件宏定义
AS_DEFS += 

# 汇编头文件目录
AS_INCLUDES +=

# 汇编源文件（starup）
ASM_SOURCES += ./sys/startup_ch32v30x_D8C.s

# 链接库
LIBS += -lc -lm -lnosys
# 库文件路径
LIBDIR += 

#######################################
# 编译器指定
#######################################
PREFIX = riscv32-wch-elf-
# 启用下一项以指定GCC目录
ifeq ($(OS),Darwin)
GCC_PATH = /Applications/MounRiver Studio 2.app/Contents/Resources/app/resources/darwin/components/WCH/Toolchain/RISC-V Embedded GCC15/bin
PYTHON_PTAH = python3
else ifeq ($(OS),Linux)
PYTHON_PTAH = python
else ifeq ($(OS),Windows_NT)
PYTHON_PTAH = python
# GCC_PATH = C:/MounRiver/MounRiver_Studio2/resources/app/resources/win32/components/WCH/Toolchain/RISC-V Embedded GCC15/bin
# GCC_PATH = D:/_tools/MRS/MounRiver_Studio2/resources/app/resources/win32/components/WCH/Toolchain/RISC-V Embedded GCC15/bin
else
$(error Unsupported OS)
endif

ifdef GCC_PATH
CC = "$(GCC_PATH)/$(PREFIX)gcc"
AS = "$(GCC_PATH)/$(PREFIX)gcc" -x assembler-with-cpp
CP = "$(GCC_PATH)/$(PREFIX)objcopy"
DUMP = "$(GCC_PATH)/$(PREFIX)objdump"
SZ = "$(GCC_PATH)/$(PREFIX)size"
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
DUMP = $(PREFIX)objdump
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# 目标单片机配置信息
#######################################
# cpu
CPU = -march=rv32imacxw

# fpu
FPU = 

# float-abi
FLOAT-ABI = -mabi=ilp32

# mcu
MCU = $(CPU) $(FPU) $(FLOAT-ABI)

# -mcmodel=medany

# link script
LDSCRIPT = ./sys/CH32V305FB.ld



# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) $(LTO) -Wall -fdata-sections -ffunction-sections

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) $(LTO) -Wall -ffunction-sections -fdata-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2 
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################

# libraries
LDFLAGS = $(MCU) --specs=nano.specs --specs=nosys.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) \
-Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -msmall-data-limit=8 \
-msave-restore -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -nostartfiles $(LTO) -Xlinker --print-memory-usage

# default action: build all
all: $(EXEC_DIR)/$(TARGET).elf $(EXEC_DIR)/$(TARGET).hex $(EXEC_DIR)/$(TARGET).bin POST_BUILD


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	@echo "[CC]    $<"
	@$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@echo "[AS]    $<"
	@$(AS) -c $(ASFLAGS) $< -o $@

$(EXEC_DIR)/$(TARGET).elf: $(OBJECTS) Makefile | $(EXEC_DIR)
	@echo "[LD]    $@"
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(EXEC_DIR)/%.hex: $(EXEC_DIR)/%.elf | $(EXEC_DIR)
	@echo "[HEX]   $< -> $@"
	@$(HEX) $< $@
	
$(EXEC_DIR)/%.bin: $(EXEC_DIR)/%.elf | $(EXEC_DIR)
	@echo "[BIN]   $< -> $@"
	@$(BIN) $< $@
	
$(BUILD_DIR):
	@mkdir $@

$(EXEC_DIR):
	@mkdir $@

#######################################
# POST_BUILD
#######################################
.PHONY: POST_BUILD
POST_BUILD: $(EXEC_DIR)/$(TARGET).elf $(EXEC_DIR)/$(TARGET).bin
	@echo "[DUMP]  $< -> $(EXEC_DIR)/$(TARGET).s"
	@$(DUMP) -d $< > $(EXEC_DIR)/$(TARGET).s
	@echo "[PACK]  $(EXEC_DIR)/$(TARGET).bin -> $(EXEC_DIR)/$(TARGET)_pack.bin"
	@$(PYTHON_PTAH) proj/pack.py make

	@echo "Build Finish"

#######################################
# 清除中间文件
#######################################
.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)
	@echo "Clean Build Finish"

.PHONY: cleanall
cleanall: clean
	@rm -rf $(EXEC_DIR)
	@echo "Clean Exec Finish"
	

#######################################
# 依赖文件
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***