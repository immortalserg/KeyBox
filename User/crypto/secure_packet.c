#include "secure_packet.h"
#include "chacha20_poly1305.h"
#include "rng.h"
#include <string.h>

static uint8_t g_session_key[32];

// Инициализация сессионного ключа
void secure_init(const uint8_t session_key[32]) {
    memcpy(g_session_key, session_key, 32);
}

// Генерация случайного nonce
void secure_generate_nonce(uint8_t nonce[SECURE_NONCE_SIZE]) {
    rng_get((uint32_t *)nonce, SECURE_NONCE_SIZE / 4);
}

// Расшифровка пакета
int secure_decrypt_packet(
    const uint8_t *packet, size_t packet_len,
    uint8_t *plaintext, size_t *pt_len,
    const uint8_t *aad, size_t aad_len
) {
    if (packet_len < SECURE_NONCE_SIZE + SECURE_TAG_SIZE)
        return -1;

    const uint8_t *nonce = packet;
    const uint8_t *ct    = packet + SECURE_NONCE_SIZE;
    size_t ct_len        = packet_len - SECURE_NONCE_SIZE - SECURE_TAG_SIZE;
    const uint8_t *tag   = packet + packet_len - SECURE_TAG_SIZE;

    int ret = chacha20_poly1305_decrypt(
        g_session_key,
        nonce,
        aad, aad_len,
        ct, ct_len,
        tag,
        plaintext
    );

    if (ret == 0) *pt_len = ct_len;

    return ret;
}

// Шифрование пакета
int secure_encrypt_packet(
    const uint8_t *plaintext, size_t pt_len,
    uint8_t *packet, size_t *packet_len,
    const uint8_t *aad, size_t aad_len
) {
    uint8_t nonce[SECURE_NONCE_SIZE];
    uint8_t tag[SECURE_TAG_SIZE];

    secure_generate_nonce(nonce);

    memcpy(packet, nonce, SECURE_NONCE_SIZE);

    int ret = chacha20_poly1305_encrypt(
        g_session_key,
        nonce,
        aad, aad_len,
        plaintext, pt_len,
        packet + SECURE_NONCE_SIZE,
        tag
    );

    if (ret != 0) return ret;

    // Append tag
    memcpy(packet + SECURE_NONCE_SIZE + pt_len, tag, SECURE_TAG_SIZE);
    *packet_len = SECURE_NONCE_SIZE + pt_len + SECURE_TAG_SIZE;

    return 0;
}
