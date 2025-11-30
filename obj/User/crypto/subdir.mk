################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/crypto/chacha20.c \
../User/crypto/chacha20_poly1305.c \
../User/crypto/crypto_x25519.c \
../User/crypto/curve25519.c \
../User/crypto/ed25519_to_x25519.c \
../User/crypto/hkdf.c \
../User/crypto/hmac.c \
../User/crypto/poly1305.c \
../User/crypto/secure_packet.c \
../User/crypto/sha256.c 

C_DEPS += \
./User/crypto/chacha20.d \
./User/crypto/chacha20_poly1305.d \
./User/crypto/crypto_x25519.d \
./User/crypto/curve25519.d \
./User/crypto/ed25519_to_x25519.d \
./User/crypto/hkdf.d \
./User/crypto/hmac.d \
./User/crypto/poly1305.d \
./User/crypto/secure_packet.d \
./User/crypto/sha256.d 

OBJS += \
./User/crypto/chacha20.o \
./User/crypto/chacha20_poly1305.o \
./User/crypto/crypto_x25519.o \
./User/crypto/curve25519.o \
./User/crypto/ed25519_to_x25519.o \
./User/crypto/hkdf.o \
./User/crypto/hmac.o \
./User/crypto/poly1305.o \
./User/crypto/secure_packet.o \
./User/crypto/sha256.o 

DIR_OBJS += \
./User/crypto/*.o \

DIR_DEPS += \
./User/crypto/*.d \

DIR_EXPANDS += \
./User/crypto/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
User/crypto/%.o: ../User/crypto/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/Debug" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/Core" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/User" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/Peripheral/inc" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/include" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/Common" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/GCC/RISC-V" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/FreeRTOS/portable/MemMang" -I"/home/immortal/Загрузки/keybox-firmware-main/code/mcu-app/User/ed25519" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

