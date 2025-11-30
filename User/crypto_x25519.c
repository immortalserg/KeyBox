#include "crypto_x25519.h"
#include "sha256.h"

// ---- TweetNaCl / Curve25519 scalar mult ----
extern void crypto_scalarmult_curve25519(uint8_t out[32],
                                         const uint8_t scalar[32],
                                         const uint8_t pub[32]);

// ---- Ed25519 -> X25519 конвертация ----
// (Используем конвертацию Монтгомери/Эдвардса)
static void ed25519_to_x25519_private(uint8_t x_sk[32], const uint8_t ed_sk[32]) {
    uint8_t h[64];
    sha512(ed_sk, 32, h);

    h[0]  &= 248;
    h[31] &= 127;
    h[31] |= 64;

    memcpy(x_sk, h, 32);
}

static void ed25519_to_x25519_public(uint8_t out[32], const uint8_t ed_pk[32]) {
    // Montgomery conversion using formula:
    // x = (1 + y) / (1 - y)  mod p
    // (чистая и простая имплементация)
    // Здесь можно вставить готовую реализацию — для примера stub:
    crypto_ed25519_pk_to_x25519(out, ed_pk);
}

// ---- ChaCha20-Poly1305 ----
#include "chacha20poly1305.h"

// ===== Derive ChaCha20 key using HKDF-SHA256 =====
static void hkdf_sha256(
    uint8_t out[32],
    const uint8_t *secret, size_t secret_len,
    const uint8_t *salt, size_t salt_len,
    const uint8_t *info, size_t info_len
) {
    uint8_t prk[32];

    // PRK = HMAC(salt, secret)
    hmac_sha256(salt, salt_len, secret, secret_len, prk);

    // OKM = HMAC(PRK, info || 0x01)
    uint8_t block[info_len + 1];
    memcpy(block, info, info_len);
    block[info_len] = 1;

    hmac_sha256(prk, 32, block, sizeof(block), out);
}

// ====== Generate shared secret ======
void crypto_x25519_shared(
    uint8_t shared_key[32],
    const uint8_t ed_sk[64],
    const uint8_t peer_ed_pk[32]
) {
    uint8_t x_sk[32];
    uint8_t x_pk[32];

    // Convert Ed25519 → X25519
    ed25519_to_x25519_private(x_sk, ed_sk);
    ed25519_to_x25519_public(x_pk, peer_ed_pk);

    // DH
    crypto_scalarmult_curve25519(shared_key, x_sk, x_pk);
}

// ====== AEAD Encrypt ======
int crypto_encrypt(
    uint8_t *out,
    const uint8_t *plaintext, uint16_t plaintext_len,
    const uint8_t shared[32]
) {
    uint8_t key[32];
    uint8_t nonce[12] = {0};

    hkdf_sha256(key, shared, 32, NULL, 0, (uint8_t*)"BLEv1", 5);

    return chacha20poly1305_encrypt(out, plaintext, plaintext_len, NULL, 0, nonce, key);
}

// ====== AEAD Decrypt ======
int crypto_decrypt(
    uint8_t *out,
    const uint8_t *ciphertext, uint16_t ciphertext_len,
    const uint8_t shared[32]
) {
    uint8_t key[32];
    uint8_t nonce[12] = {0};

    hkdf_sha256(key, shared, 32, NULL, 0, (uint8_t*)"BLEv1", 5);

    return chacha20poly1305_decrypt(out, ciphertext, ciphertext_len, NULL, 0, nonce, key);
}
