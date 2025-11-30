################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/ble.c \
../User/button.c \
../User/ch32v30x_it.c \
../User/crypto_x25519.c \
../User/flash.c \
../User/keyfob.c \
../User/led.c \
../User/main.c \
../User/rng.c \
../User/rs485.c \
../User/rtc.c \
../User/system_ch32v30x.c 

C_DEPS += \
./User/ble.d \
./User/button.d \
./User/ch32v30x_it.d \
./User/crypto_x25519.d \
./User/flash.d \
./User/keyfob.d \
./User/led.d \
./User/main.d \
./User/rng.d \
./User/rs485.d \
./User/rtc.d \
./User/system_ch32v30x.d 

OBJS += \
./User/ble.o \
./User/button.o \
./User/ch32v30x_it.o \
./User/crypto_x25519.o \
./User/flash.o \
./User/keyfob.o \
./User/led.o \
./User/main.o \
./User/rng.o \
./User/rs485.o \
./User/rtc.o \
./User/system_ch32v30x.o 

DIR_OBJS += \
./User/*.o \

DIR_DEPS += \
./User/*.d \

DIR_EXPANDS += \
./User/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
User/%.o: ../User/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/Debug" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/Core" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/User" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/Peripheral/inc" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/include" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/Common" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/GCC/RISC-V" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/MemMang" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/User/ed25519" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

