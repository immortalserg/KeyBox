#include "poly1305.h"
#include <string.h>

// Load little-endian 32-bit
static inline uint32_t load32(const uint8_t *p) {
    return ((uint32_t)p[0])
        | ((uint32_t)p[1] << 8)
        | ((uint32_t)p[2] << 16)
        | ((uint32_t)p[3] << 24);
}

// Store little-endian 32-bit
static inline void store32(uint8_t *p, uint32_t v) {
    p[0] = v & 0xff;
    p[1] = (v >> 8) & 0xff;
    p[2] = (v >> 16) & 0xff;
    p[3] = (v >> 24) & 0xff;
}

void poly1305_init(poly1305_ctx *ctx, const uint8_t key[32]) {
    memset(ctx, 0, sizeof(*ctx));

    // r (masked)
    uint64_t t0 = load32(key + 0);
    uint64_t t1 = load32(key + 4);
    uint64_t t2 = load32(key + 8);
    uint64_t t3 = load32(key + 12);

    // clamp r
    ctx->r[0] = (uint32_t)( t0                    & 0x3ffffff );
    ctx->r[1] = (uint32_t)(((t0 >> 26) | (t1 << 6)) & 0x3ffff03 );
    ctx->r[2] = (uint32_t)(((t1 >> 20) | (t2 <<12)) & 0x3ffc0ff );
    ctx->r[3] = (uint32_t)(((t2 >> 14) | (t3 <<18)) & 0x3f03fff );
    ctx->r[4] = (uint32_t)(( t3 >> 8 )             & 0x00fffff );

    // pad = s
    ctx->pad[0] = load32(key + 16);
    ctx->pad[1] = load32(key + 20);
    ctx->pad[2] = load32(key + 24);
    ctx->pad[3] = load32(key + 28);

    ctx->buffer_used = 0;
    ctx->finished    = 0;
}

static void poly1305_block(poly1305_ctx *ctx, const uint8_t block[16], int final) {
    const uint32_t hibit = final ? 0 : (1 << 24);  // 1<<24 for non-final block

    // decode block to 5 limbs (26-bit)
    uint64_t t0 = load32(block + 0);
    uint64_t t1 = load32(block + 4);
    uint64_t t2 = load32(block + 8);
    uint64_t t3 = load32(block + 12);

    uint32_t m0 = (uint32_t)( t0                     & 0x3ffffff );
    uint32_t m1 = (uint32_t)(((t0 >> 26) | t1 << 6 ) & 0x3ffffff );
    uint32_t m2 = (uint32_t)(((t1 >> 20) | t2 <<12 ) & 0x3ffffff );
    uint32_t m3 = (uint32_t)(((t2 >> 14) | t3 <<18 ) & 0x3ffffff );
    uint32_t m4 = (uint32_t)(( t3 >>  8 )            & 0x00fffff ) | hibit;

    // add block to h
    uint64_t h0 = ctx->h[0] + m0;
    uint64_t h1 = ctx->h[1] + m1;
    uint64_t h2 = ctx->h[2] + m2;
    uint64_t h3 = ctx->h[3] + m3;
    uint64_t h4 = ctx->h[4] + m4;

    // r
    uint64_t r0 = ctx->r[0];
    uint64_t r1 = ctx->r[1];
    uint64_t r2 = ctx->r[2];
    uint64_t r3 = ctx->r[3];
    uint64_t r4 = ctx->r[4];

    // multiplication (h = h * r)
    uint64_t d0 = h0*r0 + h1*r4*5 + h2*r3*5 + h3*r2*5 + h4*r1*5;
    uint64_t d1 = h0*r1 + h1*r0   + h2*r4*5 + h3*r3*5 + h4*r2*5;
    uint64_t d2 = h0*r2 + h1*r1   + h2*r0   + h3*r4*5 + h4*r3*5;
    uint64_t d3 = h0*r3 + h1*r2   + h2*r1   + h3*r0   + h4*r4*5;
    uint64_t d4 = h0*r4 + h1*r3   + h2*r2   + h3*r1   + h4*r0;

    // carry propagation
    uint64_t c;

    c = (d0 >> 26); ctx->h[0] = (uint32_t)(d0 & 0x3ffffff);
    d1 += c;

    c = (d1 >> 26); ctx->h[1] = (uint32_t)(d1 & 0x3ffffff);
    d2 += c;

    c = (d2 >> 26); ctx->h[2] = (uint32_t)(d2 & 0x3ffffff);
    d3 += c;

    c = (d3 >> 26); ctx->h[3] = (uint32_t)(d3 & 0x3ffffff);
    d4 += c;

    c = (d4 >> 26); ctx->h[4] = (uint32_t)(d4 & 0x3ffffff);
    ctx->h[0] += (uint32_t)(c * 5);

    ctx->h[1] += (ctx->h[0] >> 26);
    ctx->h[0] &= 0x3ffffff;
}

void poly1305_update(poly1305_ctx *ctx, const uint8_t *m, size_t bytes) {
    size_t i = 0;

    if (ctx->buffer_used) {
        size_t needed = 16 - ctx->buffer_used;
        if (bytes < needed) {
            memcpy(ctx->buffer + ctx->buffer_used, m, bytes);
            ctx->buffer_used += bytes;
            return;
        }

        memcpy(ctx->buffer + ctx->buffer_used, m, needed);
        poly1305_block(ctx, ctx->buffer, 0);
        ctx->buffer_used = 0;

        i += needed;
    }

    for (; i + 16 <= bytes; i += 16) {
        poly1305_block(ctx, m + i, 0);
    }

    if (i < bytes) {
        ctx->buffer_used = bytes - i;
        memcpy(ctx->buffer, m + i, ctx->buffer_used);
    }
}

void poly1305_finish(poly1305_ctx *ctx, uint8_t mac[16]) {
    if (ctx->finished) return;

    if (ctx->buffer_used) {
        // Pad last block with 1 then zeros
        ctx->buffer[ctx->buffer_used] = 1;
        for (size_t i = ctx->buffer_used + 1; i < 16; i++)
            ctx->buffer[i] = 0;

        poly1305_block(ctx, ctx->buffer, 1);
    }

    // fully carry propagate
    uint64_t h0 = ctx->h[0];
    uint64_t h1 = ctx->h[1];
    uint64_t h2 = ctx->h[2];
    uint64_t h3 = ctx->h[3];
    uint64_t h4 = ctx->h[4];

    h1 += (h0 >> 26); h0 &= 0x3ffffff;
    h2 += (h1 >> 26); h1 &= 0x3ffffff;
    h3 += (h2 >> 26); h2 &= 0x3ffffff;
    h4 += (h3 >> 26); h3 &= 0x3ffffff;

    // Compare with P = 2^130 - 5
    uint64_t g0 = h0 + 5;
    uint64_t g1 = h1 + (g0 >> 26); g0 &= 0x3ffffff;
    uint64_t g2 = h2 + (g1 >> 26); g1 &= 0x3ffffff;
    uint64_t g3 = h3 + (g2 >> 26); g2 &= 0x3ffffff;
    uint64_t g4 = h4 + (g3 >> 26) - (1ULL << 26);
    g3 &= 0x3ffffff;

    uint32_t mask = (uint32_t)((g4 >> 31) - 1);

    h0 = (h0 & ~mask) | (g0 & mask);
    h1 = (h1 & ~mask) | (g1 & mask);
    h2 = (h2 & ~mask) | (g2 & mask);
    h3 = (h3 & ~mask) | (g3 & mask);
    h4 = (h4 & ~mask) | (g4 & mask);

    // produce MAC = h + pad
    uint64_t f0 = ((h0      ) | (h1 << 26)) + ctx->pad[0];
    uint64_t f1 = ((h1 >> 6 ) | (h2 << 20)) + ctx->pad[1];
    uint64_t f2 = ((h2 >> 12) | (h3 << 14)) + ctx->pad[2];
    uint64_t f3 = ((h3 >> 18) | (h4 << 8 )) + ctx->pad[3];

    store32(mac + 0,  (uint32_t)f0);
    store32(mac + 4,  (uint32_t)f1);
    store32(mac + 8,  (uint32_t)f2);
