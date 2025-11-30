#pragma once
#include <stdint.h>

void ed25519_pk_to_x25519(uint8_t x25519_pk[32],
                          const uint8_t ed25519_pk[32]);

void ed25519_sk_to_x25519(uint8_t x25519_sk[32],
                          const uint8_t ed25519_sk[64]);
