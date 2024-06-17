// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "opensslhandler.h"

#include <openssl/pem.h>

using namespace daemonplugin_stringdecrypt;

OpenSSLHandler *OpenSSLHandler::instance()
{
    static OpenSSLHandler ins;
    return &ins;
}

void OpenSSLHandler::initKeyPairs()
{
    if (rsa)
        return;
    rsa = RSA_generate_key(2048, RSA_F4, nullptr, nullptr);

    BIO *bioPrivKey = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPrivateKey(bioPrivKey, rsa, nullptr, nullptr, 0, nullptr, nullptr);
    char *privKeyBuf;
    long privKeyLen = BIO_get_mem_data(bioPrivKey, &privKeyBuf);
    auto privKey = QByteArray(privKeyBuf, privKeyLen);
    BIO_free(bioPrivKey);

    BIO *bioPubKey = BIO_new(BIO_s_mem());
    PEM_write_bio_RSA_PUBKEY(bioPubKey, rsa);
    char *pubKeyBuf;
    long pubKeyLen = BIO_get_mem_data(bioPubKey, &pubKeyBuf);
    auto pubKey = QByteArray(pubKeyBuf, pubKeyLen);
    BIO_free(bioPubKey);

    keys = { pubKey, privKey };
}

QString OpenSSLHandler::pubKey() const
{
    return keys.first;
}

int OpenSSLHandler::decrypt(const QString &in, QString *out)
{
    Q_ASSERT(rsa);
    Q_ASSERT(out);

    QByteArray cipher = QByteArray::fromBase64(in.toLocal8Bit());

    int rsaSize = RSA_size(rsa);
    unsigned char *decrypted = new unsigned char[rsaSize];
    int decryptedLen = RSA_private_decrypt(cipher.length(),
                                           reinterpret_cast<const unsigned char *>(cipher.data()),
                                           decrypted,
                                           rsa,
                                           RSA_PKCS1_PADDING);

    if (decryptedLen == -1) {
        delete[] decrypted;
        return -1;
    }

    QByteArray source(reinterpret_cast<char *>(decrypted), decryptedLen);
    *out = QString(source);
    delete[] decrypted;
    return 0;
}

OpenSSLHandler::OpenSSLHandler(QObject *parent)
{
}

OpenSSLHandler::~OpenSSLHandler()
{
    if (rsa)
        RSA_free(rsa);
    rsa = nullptr;
}
