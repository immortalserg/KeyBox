# СТРУКТУРА МОДУЛЯ
```
crypto/
│
├── ed25519_to_x25519.c
├── ed25519_to_x25519.h
│
├── curve25519.c
├── curve25519.h
│
├── sha256.c
├── sha256.h
│
├── hmac.c
├── hmac.h
│
├── hkdf.c
├── hkdf.h
│
├── chacha20.c
├── chacha20.h
│
├── poly1305.c
├── poly1305.h
│
├── chacha20poly1305.c
├── chacha20poly1305.h
│
├── crypto_session.c
├── crypto_session.h
```

# Сгенерировать session key:
```
crypto_session_derive(
    x25519_self_private,
    x25519_peer_public,
    session_key   // 32 bytes
);
```
# Зашифровать:
```
crypto_aead_encrypt(session_key, nonce, aad, aad_len, plaintext, pt_len,
                    ciphertext, tag);
```
# Расшифровать:
```
if (!crypto_aead_decrypt(session_key, nonce, aad, aad_len,
                         ciphertext, ct_len, tag, plaintext_out)) {
    // ERROR: tag mismatch
}
```

# МОДУЛЬ №1 — Ed25519 → X25519

конвертировать Ed25519 → X25519

# МОДУЛЬ №2 — Curve25519 (X25519)

Это чистая быстрая реализация Montgomery Ladder для микроконтроллеров.

# МОДУЛЬ №3 — SHA-256

Этот SHA-256 будет использоваться:

для HMAC-SHA256

для HKDF

для генерации session key

для AEAD AAD хеширования (если потребуется)

# МОДУЛЬ №4 — HMAC-SHA256

Он использует модуль SHA-256

Этот HMAC-SHA256 понадобится для HKDF, а HKDF — для derivation session key.

МОДУЛЬ №5 — HKDF-SHA256

Этот модуль понадобится для derivation session key из результата X25519-DH.

Стандарт: RFC 5869

Использует модуль HMAC-SHA256

# МОДУЛЬ №6: ChaCha20 (RFC 7539) (ядро + шифратор).

Он нужен для ChaCha20-Poly1305 (модуль №7 будет Poly1305 и AEAD).

Этот модуль содержит:

chacha20_block — вычисление одного блока (64 байта)

chacha20_init — подготовка состояния

chacha20_xor — шифрование/дешифрование (потоковый XOR)

НЕ содержит Poly1305 (будет в следующем модуле)

# МОДУЛЬ №7: Poly1305 (RFC 7539).

Этот модуль реализует:

poly1305_init

poly1305_update

poly1305_finish

Генерацию 16-байтового MAC

Он совместим с ChaCha20 (предыдущий модуль), то есть из ChaCha20 блока №0 позже брать:

r (16 байт → маскировать)

s (16 байт)

для AEAD ChaCha20-Poly1305.

Модуль не зависит от сторонних библиотек.

# МОДУЛЬ №7 — Poly1305

# МОДУЛЬ №8 — AEAD ChaCha20-Poly1305

Это полный, рабочий AEAD-модуль: шифрование + MAC и расшифровка + проверка MAC.

Использует ChaCha20 (модуль №6) и Poly1305 (модуль №7).

Полностью совместим с:

iOS CryptoKit

libsodium crypto_aead_chacha20poly1305_ietf()

RFC 7539 (IETF AEAD)

# МОДУЛЬ №9 secure_packet

интеграция ChaCha20-Poly1305 в KeyBox.

Этот модуль отвечает за:

Прием зашифрованного пакета от iPhone: {nonce, ciphertext, tag}

Расшифровку с проверкой MAC

Подготовку ответа: шифрование {ciphertext, tag, nonce}

Опционально: проверку Ed25519-подписи, если команда требует


