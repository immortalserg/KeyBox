#include "curve25519.h"
#include <string.h>

/* ---------- Основная арифметика поля ---------- */

static const uint32_t PRIME[8] = {
    0xFFFFFEB ,0xFFFFFFFF ,0xFFFFFFFF ,0xFFFFFFFF ,
    0xFFFFFFFF ,0xFFFFFFFF ,0xFFFFFFFF ,0x7FFFFFFF
};

static void fe_copy(uint32_t *r, const uint32_t *a) {
    for (int i = 0; i < 8; i++) r[i] = a[i];
}

static void fe_add(uint32_t *r, const uint32_t *a, const uint32_t *b) {
    uint64_t c = 0;
    for (int i = 0; i < 8; i++) {
        c = (uint64_t)a[i] + b[i] + (c >> 32);
        r[i] = (uint32_t)c;
    }
}

static void fe_sub(uint32_t *r, const uint32_t *a, const uint32_t *b) {
    uint64_t c = 0;
    for (int i = 0; i < 8; i++) {
        c = (uint64_t)a[i] - b[i] - (c >> 63);
        r[i] = (uint32_t)c;
    }
}

static void fe_mul(uint32_t *r, const uint32_t *a, const uint32_t *b) {
    uint64_t t[16] = {0};

    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            t[i + j] += (uint64_t)a[i] * b[j];

    uint64_t c = 0;
    for (int i = 0; i < 15; i++) {
        t[i] += c;
        c = t[i] >> 32;
        t[i] &= 0xffffffff;
    }
    t[15] += c;

    for (int i = 0; i < 8; i++)
        r[i] = (uint32_t)t[i];
}

static uint32_t load32(const uint8_t in[4]) {
    return (uint32_t)in[0]
        | ((uint32_t)in[1] << 8)
        | ((uint32_t)in[2] << 16)
        | ((uint32_t)in[3] << 24);
}

static void store32(uint8_t out[4], uint32_t v) {
    out[0] = v & 0xFF;
    out[1] = (v >> 8) & 0xFF;
    out[2] = (v >> 16) & 0xFF;
    out[3] = (v >> 24) & 0xFF;
}

/* ---------- Конвертация между little-endian ---------- */

static void fe_frombytes(uint32_t h[8], const uint8_t s[32]) {
    for (int i = 0; i < 8; i++)
        h[i] = load32(s + i * 4);
}

static void fe_tobytes(uint8_t s[32], const uint32_t h[8]) {
    for (int i = 0; i < 8; i++)
        store32(s + i * 4, h[i]);
}

/* ---------- Инверсия поля через pow(p-2) ---------- */

static void fe_inverse(uint32_t out[8], const uint32_t z[8]) {
    uint32_t t0[8], t1[8], t2[8], t3[8];

    fe_mul(t0, z, z);
    fe_mul(t1, t0, z);
    fe_mul(t0, t0, t1);
    fe_mul(t0, t0, t0);
    fe_mul(t0, t0, t1);
    fe_mul(out, t0, t0);

    for (int i = 0; i < 250; i++)
        fe_mul(out, out, out);

    fe_mul(out, out, t0);
}

/* ---------- Монтгомери-лесенка (X25519) ---------- */

void curve25519(uint8_t out[32],
                const uint8_t scalar[32],
                const uint8_t point[32]) {
    uint8_t e[32];
    memcpy(e, scalar, 32);

    // clamp
    e[0] &= 248;
    e[31] &= 127;
    e[31] |= 64;

    uint32_t x1[8], x2[8], z2[8], x3[8], z3[8];
    fe_frombytes(x1, point);
    memset(z2, 0, sizeof(z2));
    memset(z3, 0, sizeof(z3));
    fe_copy(x2, x1);
    z2[0] = 1;
    x3[0] = 1;

    int swap = 0;

    for (int pos = 254; pos >= 0; pos--) {
        int bit = (e[pos >> 3] >> (pos & 7)) & 1;
        swap ^= bit;

        // conditional swap
        if (swap) {
            for (int i = 0; i < 8; i++) {
                uint32_t t;
                t = x2[i]; x2[i] = x3[i]; x3[i] = t;
                t = z2[i]; z2[i] = z3[i]; z3[i] = t;
            }
        }
        swap = bit;

        uint32_t t0[8], t1[8], t2_[8], t3_[8];

        fe_add(t0, x2, z2);
        fe_sub(t1, x2, z2);
        fe_add(t2_, x3, z3);
        fe_sub(t3_, x3, z3);

        fe_mul(x2, t0, t3_);
        fe_mul(z2, t1, t2_);
        fe_mul(t2_, t2_, t2_);
        fe_mul(t3_, t3_, t3_);
        fe_mul(x3, x2, x2);
        fe_mul(z3, z2, z2);

        fe_sub(z2, t2_, t3_);
        fe_mul(z2, z2, x1);
        fe_add(x2, t2_, t3_);
    }

    uint32_t z2inv[8];
    fe_inverse(z2inv, z2);
    fe_mul(x2, x2, z2inv);
    fe_tobytes(out, x2);
}

/* ---------- Упрощённые операции для ed25519_to_x25519 ---------- */

void curve25519_add(uint8_t out[32],
                    const uint8_t a[32],
                    const uint8_t b[32]) {
    uint32_t A[8], B[8], C[8];
    fe_frombytes(A, a);
    fe_frombytes(B, b);
    fe_add(C, A, B);
    fe_tobytes(out, C);
}

void curve25519_sub(uint8_t out[32],
                    const uint8_t a[32],
                    const uint8_t b[32]) {
    uint32_t A[8], B[8], C[8];
    fe_frombytes(A, a);
    fe_frombytes(B, b);
    fe_sub(C, A, B);
    fe_tobytes(out, C);
}

void curve25519_mul(uint8_t out[32],
                    const uint8_t a[32],
                    const uint8_t b[32]) {
    uint32_t A[8], B[8], C[8];
    fe_frombytes(A, a);
    fe_frombytes(B, b);
    fe_mul(C, A, B);
    fe_tobytes(out, C);
}

void curve25519_inverse(uint8_t out[32],
                        const uint8_t z[32]) {
    uint32_t Z[8], R[8];
    fe_frombytes(Z, z);
    fe_inverse(R, Z);
    fe_tobytes(out, R);
}
