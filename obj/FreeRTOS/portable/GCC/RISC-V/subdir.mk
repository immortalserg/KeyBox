################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FreeRTOS/portable/GCC/RISC-V/port.c 

C_DEPS += \
./FreeRTOS/portable/GCC/RISC-V/port.d 

S_UPPER_SRCS += \
../FreeRTOS/portable/GCC/RISC-V/portASM.S 

S_UPPER_DEPS += \
./FreeRTOS/portable/GCC/RISC-V/portASM.d 

OBJS += \
./FreeRTOS/portable/GCC/RISC-V/port.o \
./FreeRTOS/portable/GCC/RISC-V/portASM.o 

DIR_OBJS += \
./FreeRTOS/portable/GCC/RISC-V/*.o \

DIR_DEPS += \
./FreeRTOS/portable/GCC/RISC-V/*.d \

DIR_EXPANDS += \
./FreeRTOS/portable/GCC/RISC-V/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
FreeRTOS/portable/GCC/RISC-V/%.o: ../FreeRTOS/portable/GCC/RISC-V/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/Debug" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/Core" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/User" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/Peripheral/inc" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/include" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/Common" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/GCC/RISC-V" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/MemMang" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/User/ed25519" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

FreeRTOS/portable/GCC/RISC-V/%.o: ../FreeRTOS/portable/GCC/RISC-V/%.S
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -x assembler-with-cpp -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/Startup" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/include" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/Common" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/GCC/RISC-V" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/MemMang" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

