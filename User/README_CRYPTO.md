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
# МОДУЛЬ №2 — Curve25519 (X25519)
# МОДУЛЬ №3 — SHA-256

Этот SHA-256 будет использоваться:

для HMAC-SHA256

для HKDF

для генерации session key

для AEAD AAD хеширования (если потребуется)



