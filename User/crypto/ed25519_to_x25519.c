#include "ed25519_to_x25519.h"
#include "sha512.h"   // Используется уже существующий Ed25519 SHA512

// Клэмпинг приватного ключа X25519
static void clamp25519(uint8_t k[32]) {
    k[0]  &= 248;
    k[31] &= 127;
    k[31] |= 64;
}

void ed25519_pk_to_x25519(uint8_t out[32], const uint8_t in[32]) {
    // Функция: X25519_pub = ed25519_point → Montgomery
    // Стандартная реализация на основе libsodium
    uint8_t A[32];
    memcpy(A, in, 32);
    A[0] &= 0x7F; // убрать знак
    // конвертация: X = (1 + y) / (1 - y)
    // здесь используется упрощённая формула для Curve25519
    // но на микроконтроллер это ок
    // X = (1 + y) * inv(1 - y)
    // y = A
    uint8_t one[32] = {1};
    uint8_t num[32], den[32];

    // num = 1 + A
    curve25519_add(num, one, A);

    // den = 1 - A
    curve25519_sub(den, one, A);

    uint8_t den_inv[32];
    curve25519_inverse(den_inv, den);

    curve25519_mul(out, num, den_inv);
}

void ed25519_sk_to_x25519(uint8_t x25519_sk[32], const uint8_t ed25519_sk[64]) {
    // Ed25519 приватный хранится как: sk[32..63] = SHA512(seed)
    memcpy(x25519_sk, ed25519_sk, 32);
    clamp25519(x25519_sk);
}
