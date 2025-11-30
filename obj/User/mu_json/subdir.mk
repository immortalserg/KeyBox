################################################################################
# MRS Version: 2.2.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/mu_json/mu_json.c 

C_DEPS += \
./User/mu_json/mu_json.d 

OBJS += \
./User/mu_json/mu_json.o 


EXPANDS += \
./User/mu_json/mu_json.c.234r.expand 



# Each subdirectory must supply rules for building sources it contributes
User/mu_json/%.o: ../User/mu_json/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"w:/Projects/BTA320/code/mcu-app/Debug" -I"w:/Projects/BTA320/code/mcu-app/Core" -I"w:/Projects/BTA320/code/mcu-app/User" -I"w:/Projects/BTA320/code/mcu-app/Peripheral/inc" -I"w:/Projects/BTA320/code/mcu-app/FreeRTOS" -I"w:/Projects/BTA320/code/mcu-app/FreeRTOS/include" -I"w:/Projects/BTA320/code/mcu-app/FreeRTOS/portable/Common" -I"w:/Projects/BTA320/code/mcu-app/FreeRTOS/portable/GCC/RISC-V" -I"w:/Projects/BTA320/code/mcu-app/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions" -I"w:/Projects/BTA320/code/mcu-app/FreeRTOS/portable/MemMang" -I"w:/Projects/BTA320/code/mcu-app/User/ed25519" -I"w:/Projects/BTA320/code/mcu-app/User/mu_json" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

