#pragma once
#include <stdint.h>

// Выполняет X25519(dh) = scalar * point
void curve25519(uint8_t out[32],
                const uint8_t scalar[32],
                const uint8_t point[32]);

// Служебные (используются модулем ed25519_to_x25519)
void curve25519_add(uint8_t out[32],
                    const uint8_t a[32],
                    const uint8_t b[32]);

void curve25519_sub(uint8_t out[32],
                    const uint8_t a[32],
                    const uint8_t b[32]);

void curve25519_mul(uint8_t out[32],
                    const uint8_t a[32],
                    const uint8_t b[32]);

void curve25519_inverse(uint8_t out[32],
                        const uint8_t z[32]);
