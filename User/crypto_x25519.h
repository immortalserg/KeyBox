#pragma once
#include <stdint.h>
#include <string.h>

void crypto_x25519_shared(
    uint8_t shared_key[32],
    const uint8_t ed_sk[64],
    const uint8_t peer_ed_pk[32]
);

int crypto_encrypt(
    uint8_t *out,
    const uint8_t *plaintext,
    uint16_t plaintext_len,
    const uint8_t shared[32]
);

int crypto_decrypt(
    uint8_t *out,
    const uint8_t *ciphertext,
    uint16_t ciphertext_len,
    const uint8_t shared[32]
);
