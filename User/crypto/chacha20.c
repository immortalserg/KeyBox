#include "chacha20.h"
#include <string.h>

#define ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))

#define QR(a,b,c,d) ( \
    a += b, d ^= a, d = ROTL(d,16), \
    c += d, b ^= c, b = ROTL(b,12), \
    a += b, d ^= a, d = ROTL(d, 8), \
    c += d, b ^= c, b = ROTL(b, 7)  \
)

static const char *sigma = "expand 32-byte k";

// Инициализация состояния ChaCha20
void chacha20_init(
    chacha20_ctx *ctx,
    const uint8_t key[32],
    uint32_t counter,
    const uint8_t nonce[12]
) {
    ctx->state[0]  = ((uint32_t *)sigma)[0];
    ctx->state[1]  = ((uint32_t *)sigma)[1];
    ctx->state[2]  = ((uint32_t *)sigma)[2];
    ctx->state[3]  = ((uint32_t *)sigma)[3];

    // key
    for (int i = 0; i < 8; i++) {
        ctx->state[4 + i] = ((uint32_t *)key)[i];
    }

    ctx->state[12] = counter;
    ctx->state[13] = ((uint32_t *)nonce)[0];
    ctx->state[14] = ((uint32_t *)nonce)[1];
    ctx->state[15] = ((uint32_t *)nonce)[2];
}

// Генерация 64-байтового блока
void chacha20_block(const chacha20_ctx *ctx, uint8_t output[64]) {
    uint32_t x[16];
    memcpy(x, ctx->state, sizeof(x));

    for (int i = 0; i < 10; i++) {
        // 20 rounds = 10 iterations of column + diagonal
        // Column rounds
        QR(x[0], x[4], x[8],  x[12]);
        QR(x[1], x[5], x[9],  x[13]);
        QR(x[2], x[6], x[10], x[14]);
        QR(x[3], x[7], x[11], x[15]);

        // Diagonal rounds
        QR(x[0], x[5], x[10], x[15]);
        QR(x[1], x[6], x[11], x[12]);
        QR(x[2], x[7], x[8],  x[13]);
        QR(x[3], x[4], x[9],  x[14]);
    }

    for (int i = 0; i < 16; i++) {
        x[i] += ctx->state[i];
    }

    memcpy(output, x, 64);
}

// XOR-шифрование / расшифрование
void chacha20_xor(
    chacha20_ctx *ctx,
    uint8_t *data,
    size_t len
) {
    uint8_t keystream[64];
    size_t offset = 0;

    while (offset < len) {
        chacha20_block(ctx, keystream);

        size_t block_len = (len - offset > 64 ? 64 : (len - offset));

        for (size_t i = 0; i < block_len; i++) {
            data[offset + i] ^= keystream[i];
        }

        ctx->state[12]++; // increment counter
        offset += block_len;
    }
}
