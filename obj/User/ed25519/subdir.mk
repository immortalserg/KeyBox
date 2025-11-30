################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/ed25519/add_scalar.c \
../User/ed25519/fe.c \
../User/ed25519/ge.c \
../User/ed25519/key_exchange.c \
../User/ed25519/keypair.c \
../User/ed25519/sc.c \
../User/ed25519/seed.c \
../User/ed25519/sha512.c \
../User/ed25519/sign.c \
../User/ed25519/verify.c 

C_DEPS += \
./User/ed25519/add_scalar.d \
./User/ed25519/fe.d \
./User/ed25519/ge.d \
./User/ed25519/key_exchange.d \
./User/ed25519/keypair.d \
./User/ed25519/sc.d \
./User/ed25519/seed.d \
./User/ed25519/sha512.d \
./User/ed25519/sign.d \
./User/ed25519/verify.d 

OBJS += \
./User/ed25519/add_scalar.o \
./User/ed25519/fe.o \
./User/ed25519/ge.o \
./User/ed25519/key_exchange.o \
./User/ed25519/keypair.o \
./User/ed25519/sc.o \
./User/ed25519/seed.o \
./User/ed25519/sha512.o \
./User/ed25519/sign.o \
./User/ed25519/verify.o 

DIR_OBJS += \
./User/ed25519/*.o \

DIR_DEPS += \
./User/ed25519/*.d \

DIR_EXPANDS += \
./User/ed25519/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
User/ed25519/%.o: ../User/ed25519/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/Debug" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/Core" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/User" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/Peripheral/inc" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/include" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/Common" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/GCC/RISC-V" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/MemMang" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/User/ed25519" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

