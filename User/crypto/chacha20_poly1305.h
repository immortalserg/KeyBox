#pragma once
#include <stdint.h>
#include <stddef.h>

#define CHACHA20_POLY1305_KEY_SIZE   32
#define CHACHA20_POLY1305_NONCE_SIZE 12
#define CHACHA20_POLY1305_TAG_SIZE   16

// AEAD Encrypt
// out = ciphertext, tag appended separately
int chacha20_poly1305_encrypt(
    const uint8_t key[32],
    const uint8_t nonce[12],
    const uint8_t *aad,  size_t aad_len,
    const uint8_t *pt,   size_t pt_len,
    uint8_t *ct,
    uint8_t tag[16]
);

// AEAD Decrypt
// returns 0 = OK, -1 = TAG FAIL
int chacha20_poly1305_decrypt(
    const uint8_t key[32],
    const uint8_t nonce[12],
    const uint8_t *aad,  size_t aad_len,
    const uint8_t *ct,   size_t ct_len,
    const uint8_t tag[16],
    uint8_t *pt
);
