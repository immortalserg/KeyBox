#pragma once
#include <stdint.h>
#include <stddef.h>

#define HMAC_SHA256_SIZE 32

void hmac_sha256(
    const uint8_t *key, size_t key_len,
    const uint8_t *data, size_t data_len,
    uint8_t out[HMAC_SHA256_SIZE]
);
