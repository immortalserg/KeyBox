################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Debug/debug.c 

C_DEPS += \
./Debug/debug.d 

OBJS += \
./Debug/debug.o 

DIR_OBJS += \
./Debug/*.o \

DIR_DEPS += \
./Debug/*.d \

DIR_EXPANDS += \
./Debug/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
Debug/%.o: ../Debug/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/Debug" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/Core" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/User" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/Peripheral/inc" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/include" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/Common" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/GCC/RISC-V" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/MemMang" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/User/ed25519" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

