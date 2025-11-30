################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################
-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include User/ed25519/subdir.mk
-include User/subdir.mk
-include Startup/subdir.mk
-include Peripheral/src/subdir.mk
-include FreeRTOS/portable/MemMang/subdir.mk
-include FreeRTOS/portable/GCC/RISC-V/subdir.mk
-include FreeRTOS/subdir.mk
-include Debug/subdir.mk
-include Core/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
mcu-app.hex \

SECONDARY_LIST += \
mcu-app.lst \

SECONDARY_SIZE += \
mcu-app.siz \


# All Target
all: 
	$(MAKE) --no-print-directory main-build 

main-build: mcu-app.elf secondary-outputs

# Tool invocations
mcu-app.elf: $(OBJS) $(USER_OBJS_ESCAPE)
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -T "/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/Ld/Link.ld" -nostartfiles -Xlinker --gc-sections -Wl,-Map,"mcu-app.map" --specs=nano.specs --specs=nosys.specs -o "mcu-app.elf" $(OBJS) $(LIBS)
mcu-app.hex: mcu-app.elf
	@	riscv-none-embed-objcopy -O ihex "mcu-app.elf" "mcu-app.hex"
mcu-app.lst: mcu-app.elf
	@	riscv-none-embed-objdump --all-headers --demangle --disassemble -M xw "mcu-app.elf" > "mcu-app.lst"
mcu-app.siz: mcu-app.elf
	riscv-none-embed-size --format=berkeley "mcu-app.elf"

# Other Targets
clean:
	-$(RM) $(DIR_OBJS) $(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE) mcu-app.elf
	-$(RM) $(DIR_EXPANDS) $(CALLGRAPH_DOT)
	-$(RM) $(DIR_DEPS)
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets