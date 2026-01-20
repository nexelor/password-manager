#include "encryption.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/err.h>
#include <QDebug>

bool Encryption::initialize() {
    return true;
}

QByteArray Encryption::generateSalt() {
    QByteArray salt(16, 0);
    RAND_bytes(reinterpret_cast<unsigned char*>(salt.data()), salt.size());
    return salt;
}

QByteArray Encryption::deriveMasterKey(const QString &masterPassword, 
                                       const QByteArray &salt) {
    QByteArray key(32, 0);
    QByteArray passwordBytes = masterPassword.toUtf8();
    
    PKCS5_PBKDF2_HMAC(passwordBytes.constData(), passwordBytes.size(),
                      reinterpret_cast<const unsigned char*>(salt.constData()),
                      salt.size(), 100000, EVP_sha256(),
                      key.size(), reinterpret_cast<unsigned char*>(key.data()));
    
    return key;
}

QByteArray Encryption::encrypt(const QByteArray &data, const QByteArray &key) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return QByteArray();

    QByteArray iv(16, 0);
    RAND_bytes(reinterpret_cast<unsigned char*>(iv.data()), iv.size());

    QByteArray encrypted;
    encrypted.resize(data.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));

    int len = 0, ciphertext_len = 0;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                          reinterpret_cast<const unsigned char*>(key.constData()),
                          reinterpret_cast<const unsigned char*>(iv.constData())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    if (EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(encrypted.data()),
                         &len, reinterpret_cast<const unsigned char*>(data.constData()),
                         data.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(encrypted.data()) + len,
                           &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    encrypted.resize(ciphertext_len);

    return iv + encrypted;
}

QByteArray Encryption::decrypt(const QByteArray &encryptedData, 
                               const QByteArray &key) {
    if (encryptedData.size() < 16) return QByteArray();

    QByteArray iv = encryptedData.left(16);
    QByteArray ciphertext = encryptedData.mid(16);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return QByteArray();

    QByteArray decrypted;
    decrypted.resize(ciphertext.size());

    int len = 0, plaintext_len = 0;

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                          reinterpret_cast<const unsigned char*>(key.constData()),
                          reinterpret_cast<const unsigned char*>(iv.constData())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    if (EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(decrypted.data()),
                         &len, reinterpret_cast<const unsigned char*>(ciphertext.constData()),
                         ciphertext.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    plaintext_len = len;

    if (EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(decrypted.data()) + len,
                           &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    decrypted.resize(plaintext_len);

    return decrypted;
}

QString Encryption::hashPassword(const QString &password) {
    QByteArray hash(32, 0);
    QByteArray passwordBytes = password.toUtf8();
    
    SHA256(reinterpret_cast<const unsigned char*>(passwordBytes.constData()),
           passwordBytes.size(),
           reinterpret_cast<unsigned char*>(hash.data()));
    
    return hash.toHex();
}

bool Encryption::verifyPassword(const QString &password, const QString &hash) {
    return hashPassword(password) == hash;
}
