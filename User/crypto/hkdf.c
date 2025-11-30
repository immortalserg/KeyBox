#include "hkdf.h"
#include "hmac.h"
#include <string.h>

// HKDF-Extract (PRK = HMAC(salt, IKM))
void hkdf_extract(
    const uint8_t *salt, size_t salt_len,
    const uint8_t *ikm,  size_t ikm_len,
    uint8_t prk[32]
) {
    if (salt == NULL || salt_len == 0) {
        // если соли нет — используем нулевую
        uint8_t zero_salt[32] = {0};
        hmac_sha256(zero_salt, 32, ikm, ikm_len, prk);
    } else {
        hmac_sha256(salt, salt_len, ikm, ikm_len, prk);
    }
}

// HKDF-Expand (OKM = T(1)||T(2)||...)
void hkdf_expand(
    const uint8_t prk[32],
    const uint8_t *info, size_t info_len,
    uint8_t *okm, size_t okm_len
) {
    uint8_t T[32];
    size_t T_len = 0;
    size_t written = 0;
    uint8_t counter = 1;

    while (written < okm_len) {
        // T(i) = HMAC(PRK, T(i-1) || info || counter)
        uint8_t buffer[32 + info_len + 1];
        size_t pos = 0;

        if (T_len > 0) {
            memcpy(buffer, T, T_len);
            pos += T_len;
        }

        if (info_len > 0) {
            memcpy(buffer + pos, info, info_len);
            pos += info_len;
        }

        buffer[pos] = counter;

        hmac_sha256(prk, 32, buffer, pos + 1, T);

        size_t to_copy = (okm_len - written < 32) ? (okm_len - written) : 32;
        memcpy(okm + written, T, to_copy);

        written += to_copy;
        counter++;
        T_len = 32;
    }
}

// Полная HKDF (Extract + Expand)
void hkdf_sha256(
    const uint8_t *salt, size_t salt_len,
    const uint8_t *ikm,  size_t ikm_len,
    const uint8_t *info, size_t info_len,
    uint8_t out_key[32]
) {
    uint8_t prk[32];
    hkdf_extract(salt, salt_len, ikm, ikm_len, prk);
    hkdf_expand(prk, info, info_len, out_key, 32);
}
