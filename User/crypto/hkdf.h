#pragma once
#include <stdint.h>
#include <stddef.h>

#define HKDF_SHA256_LEN 32   // длина ключей, которые мы выводим

// HKDF-Extract
void hkdf_extract(
    const uint8_t *salt, size_t salt_len,
    const uint8_t *ikm,  size_t ikm_len,
    uint8_t prk[32] // pseudo-random key
);

// HKDF-Expand
void hkdf_expand(
    const uint8_t prk[32],
    const uint8_t *info, size_t info_len,
    uint8_t *okm, size_t okm_len
);

// Полная HKDF (Extract+Expand)
void hkdf_sha256(
    const uint8_t *salt, size_t salt_len,
    const uint8_t *ikm,  size_t ikm_len,
    const uint8_t *info, size_t info_len,
    uint8_t out_key[32]
);
