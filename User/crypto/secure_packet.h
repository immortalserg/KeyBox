#pragma once
#include <stdint.h>
#include <stddef.h>

#define SECURE_NONCE_SIZE 12
#define SECURE_TAG_SIZE   16

// Инициализация secure channel с сессионным ключом
void secure_init(const uint8_t session_key[32]);

// Расшифровка пакета
// packet = nonce||ciphertext||tag
// plaintext = output buffer
// ct_len = длина ciphertext
// returns 0 = ok, -1 = auth fail
int secure_decrypt_packet(
    const uint8_t *packet, size_t packet_len,
    uint8_t *plaintext, size_t *pt_len,
    const uint8_t *aad, size_t aad_len
);

// Шифрование пакета
// packet = output buffer = nonce||ciphertext||tag
int secure_encrypt_packet(
    const uint8_t *plaintext, size_t pt_len,
    uint8_t *packet, size_t *packet_len,
    const uint8_t *aad, size_t aad_len
);

// Генерация случайного nonce
void secure_generate_nonce(uint8_t nonce[SECURE_NONCE_SIZE]);
