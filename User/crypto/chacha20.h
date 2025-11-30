#pragma once
#include <stdint.h>
#include <stddef.h>

#define CHACHA20_KEY_SIZE   32
#define CHACHA20_NONCE_SIZE 12

typedef struct {
    uint32_t state[16];
} chacha20_ctx;

// Инициализация ChaCha20: key (32), counter (32 bit), nonce (12)
void chacha20_init(
    chacha20_ctx *ctx,
    const uint8_t key[32],
    uint32_t counter,
    const uint8_t nonce[12]
);

// Генерация следующего 64-байтового блока
void chacha20_block(const chacha20_ctx *ctx, uint8_t output[64]);

// Шифрование/дешифрование (XOR)
void chacha20_xor(
    chacha20_ctx *ctx,
    uint8_t *data,
    size_t len
);
