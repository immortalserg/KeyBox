#include "hmac.h"
#include "sha256.h"
#include <string.h>

#define BLOCK_SIZE 64

void hmac_sha256(
    const uint8_t *key, size_t key_len,
    const uint8_t *data, size_t data_len,
    uint8_t out[32]
) {
    uint8_t ipad[BLOCK_SIZE];
    uint8_t opad[BLOCK_SIZE];
    uint8_t key_block[BLOCK_SIZE];
    uint8_t temp_hash[32];

    // 1. Подготовка ключа: если >64 байт — хешируем
    if (key_len > BLOCK_SIZE) {
        SHA256_CTX kctx;
        sha256_init(&kctx);
        sha256_update(&kctx, key, key_len);
        sha256_final(&kctx, key_block);
        memset(key_block + 32, 0, 32);
    } else {
        memcpy(key_block, key, key_len);
        memset(key_block + key_len, 0, BLOCK_SIZE - key_len);
    }

    // 2. Заполняем ipad и opad
    for (int i = 0; i < BLOCK_SIZE; i++) {
        ipad[i] = key_block[i] ^ 0x36;
        opad[i] = key_block[i] ^ 0x5C;
    }

    // 3. inner = SHA256(ipad || data)
    SHA256_CTX ictx;
    sha256_init(&ictx);
    sha256_update(&ictx, ipad, BLOCK_SIZE);
    sha256_update(&ictx, data, data_len);
    sha256_final(&ictx, temp_hash);

    // 4. outer = SHA256(opad || inner)
    SHA256_CTX octx;
    sha256_init(&octx);
    sha256_update(&octx, opad, BLOCK_SIZE);
    sha256_update(&octx, temp_hash, 32);
    sha256_final(&octx, out);
}
