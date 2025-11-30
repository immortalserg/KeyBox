#include "chacha20_poly1305.h"
#include "chacha20.h"
#include "poly1305.h"
#include <string.h>

// RFC 7539: Poly1305 key = first 32 bytes of ChaCha20 block(counter=0)
static void generate_poly1305_key(
    const uint8_t key[32],
    const uint8_t nonce[12],
    uint8_t poly_key[32]
) {
    chacha20_ctx ctx;
    uint8_t block0[64];

    // counter = 0
    chacha20_init(&ctx, key, 0, nonce);
    chacha20_block(&ctx, block0);

    memcpy(poly_key, block0, 32); // r||s
}

// RFC 7539: Encode 64-bit LE integer
static void store64_le(uint8_t out[8], uint64_t v) {
    for (int i = 0; i < 8; i++) out[i] = (v >> (8 * i)) & 0xFF;
}

// RFC: Poly1305 update padded with zero to 16 bytes
static void poly1305_update_padded(poly1305_ctx *ctx, const uint8_t *data, size_t len) {
    if (len == 0) return;

    poly1305_update(ctx, data, len);

    size_t r = len % 16;
    if (r != 0) {
        uint8_t zero[16] = {0};
        poly1305_update(ctx, zero, 16 - r);
    }
}


int chacha20_poly1305_encrypt(
    const uint8_t key[32],
    const uint8_t nonce[12],
    const uint8_t *aad,  size_t aad_len,
    const uint8_t *pt,   size_t pt_len,
    uint8_t *ct,
    uint8_t tag[16]
) {
    uint8_t poly_key[32];

    // 1) Poly1305 key generation
    generate_poly1305_key(key, nonce, poly_key);

    // 2) Encrypt plaintext with ChaCha20 counter = 1
    chacha20_ctx ctx;
    chacha20_init(&ctx, key, 1, nonce); // IMPORTANT: counter = 1
    memcpy(ct, pt, pt_len);
    chacha20_xor(&ctx, ct, pt_len);

    // 3) Build Poly1305 input
    poly1305_ctx pctx;
    poly1305_init(&pctx, poly_key);

    // AAD
    poly1305_update_padded(&pctx, aad, aad_len);

    // ciphertext
    poly1305_update_padded(&pctx, ct, pt_len);

    // lengths block
    uint8_t length_block[16];
    store64_le(length_block + 0, aad_len);
    store64_le(length_block + 8, pt_len);
    poly1305_update(&pctx, length_block, 16);

    // 4) Output tag
    poly1305_finish(&pctx, tag);

    return 0;
}


int chacha20_poly1305_decrypt(
    const uint8_t key[32],
    const uint8_t nonce[12],
    const uint8_t *aad,  size_t aad_len,
    const uint8_t *ct,   size_t ct_len,
    const uint8_t tag[16],
    uint8_t *pt
) {
    uint8_t poly_key[32];
    uint8_t expected_tag[16];

    // 1) Poly1305 key generation
    generate_poly1305_key(key, nonce, poly_key);

    // 2) Recompute authentication tag
    poly1305_ctx pctx;
    poly1305_init(&pctx, poly_key);

    poly1305_update_padded(&pctx, aad, aad_len);
    poly1305_update_padded(&pctx, ct, ct_len);

    uint8_t length_block[16];
    store64_le(length_block + 0, aad_len);
    store64_le(length_block + 8, ct_len);
    poly1305_update(&pctx, length_block, 16);

    poly1305_finish(&pctx, expected_tag);

    // 3) Constant-time tag compare
    uint8_t diff = 0;
    for (int i = 0; i < 16; i++)
        diff |= (expected_tag[i] ^ tag[i]);

    if (diff != 0) {
        return -1; // authentication failure
    }

    // 4) Decrypt ciphertext with ChaCha20 counter = 1
    chacha20_ctx ctx;
    chacha20_init(&ctx, key, 1, nonce);

    memcpy(pt, ct, ct_len);
    chacha20_xor(&ctx, pt, ct_len);

    return 0;
}
