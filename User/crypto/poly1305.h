#pragma once
#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t r[5];     // r (masked key part)
    uint32_t h[5];     // accumulator
    uint32_t pad[4];   // s (second key part)
    uint8_t  buffer[16];
    size_t   buffer_used;
    uint8_t  finished;
} poly1305_ctx;

// Инициализация Poly1305 ключом (32 байта: r|s)
void poly1305_init(poly1305_ctx *ctx, const uint8_t key[32]);

// Добавление данных
void poly1305_update(poly1305_ctx *ctx, const uint8_t *m, size_t bytes);

// Финализация и вывод MAC (16 байт)
void poly1305_finish(poly1305_ctx *ctx, uint8_t mac[16]);
