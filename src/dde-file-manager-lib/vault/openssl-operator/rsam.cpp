/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "rsam.h"

#include <QDebug>

#include <openssl/pem.h>
#include <openssl/err.h>

bool rsam::createPublicAndPrivateKey(QString &publicKey, QString &privateKey)

{
    // 创建rsa对象
    RSA *pRsa = RSA_new();
    BIGNUM *pNum = BN_new();
    BN_set_word(pNum, RSA_F4);
    int nRet = RSA_generate_key_ex(pRsa, KEY_LENGTH, pNum, nullptr);
    if (nRet != 1) {
        qDebug() << "RSA_generate_key_ex 失败！";
        return false;
    }

    // 创建私钥对象
    BIO *pPrivateBio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPrivateKey(pPrivateBio, pRsa, nullptr, nullptr, 0, nullptr, nullptr);

    // 创建公钥对象
    BIO *pPublicBio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPublicKey(pPublicBio, pRsa);

    // 获得私钥长度
    int nPrivateKeyLen = BIO_pending(pPrivateBio);
    // 获得公钥长度
    int nPublicKeyLen = BIO_pending(pPublicBio);

    // 获得私钥
    char *pPrivateKey = new char[nPrivateKeyLen];
    BIO_read(pPrivateBio, pPrivateKey, nPrivateKeyLen);
    // 获得公钥
    char *pPublicKey = new char[nPublicKeyLen];
    BIO_read(pPublicBio, pPublicKey, nPublicKeyLen);

    // 将私钥转化为QByte
    privateKey = QByteArray(pPrivateKey, nPrivateKeyLen);
    // 将公钥转化成QByte
    publicKey = QByteArray(pPublicKey, nPublicKeyLen);


    // 销毁对象
    RSA_free(pRsa);
    BN_free(pNum);
    BIO_free_all(pPrivateBio);
    BIO_free_all(pPublicBio);
    delete[] pPrivateKey;
    delete[] pPublicKey;

    return true;
}

// 私钥加密
QString rsam::privateKeyEncrypt(const QString &password, const QString &privateKey)
{
    // 转化私钥
    QByteArray privateKeyArry = privateKey.toUtf8();
    uchar *pPrivateKey = reinterpret_cast<uchar *>(privateKeyArry.data());
    BIO *pPrivateKeyBio = BIO_new_mem_buf(pPrivateKey, privateKey.length());
    if (pPrivateKeyBio == nullptr) {
        qDebug() << "BIO_new_mem_buf 失败！";
        return "";
    }

    RSA *pRsa = RSA_new();
    pRsa = PEM_read_bio_RSAPrivateKey(pPrivateKeyBio, &pRsa, nullptr, nullptr);
    if (pRsa == nullptr) {
        BIO_free_all(pPrivateKeyBio);
        qDebug() << "PEM_read_bio_RSAPrivateKey 失败！";
        return "";
    }

    int nLength = RSA_size(pRsa);
    char *strCiphertext = new char[nLength];
    memset(strCiphertext, 0, size_t(nLength));
    QByteArray passwordArry = password.toUtf8();
    int npasswordLen = passwordArry.length();
    uchar *pPasswordData = reinterpret_cast<uchar *>(passwordArry.data());
    int nSize = RSA_private_encrypt(npasswordLen,
                                    pPasswordData,
                                    reinterpret_cast<uchar *>(strCiphertext),
                                    pRsa,
                                    RSA_PKCS1_PADDING);

    QString qstrCiphertext = "";
    if (nSize >= 0) {
        QByteArray arry(strCiphertext, nSize);
        qstrCiphertext = arry.toBase64();
    }

    // 销毁对象
    delete[] strCiphertext;
    BIO_free_all(pPrivateKeyBio);
    RSA_free(pRsa);

    return qstrCiphertext;
}

// 公钥解密
QString rsam::publicKeyDecrypt(const QString &ciphertext, const QString &publicKey)
{
    QByteArray publickKeyArry = publicKey.toUtf8();
    uchar *pPublicKey = reinterpret_cast<uchar *>(publickKeyArry.data());
    BIO *pPublicKeyBio = BIO_new_mem_buf(pPublicKey, publicKey.length());
    if (pPublicKeyBio == nullptr) {
        qDebug() << "BIO_new_mem_buf 失败！";
        return "";
    }

    RSA *pRsa = RSA_new();
    if (publicKey.contains(KEY_BEGIN)) {
        pRsa = PEM_read_bio_RSAPublicKey(pPublicKeyBio, &pRsa, nullptr, nullptr);
    } else {
        pRsa = PEM_read_bio_RSA_PUBKEY(pPublicKeyBio, &pRsa, nullptr, nullptr);
    }

    if (!pRsa) {
        qDebug() << "PEM_read_bio_RSAPublicKey 失败！";
        return "";
    }

    int nLen = RSA_size(pRsa);
    char *passwordBuf = new char[nLen];
    memset(passwordBuf, 0, size_t(nLen));

    // 解密
    QByteArray ciphertextArry = ciphertext.toUtf8();
    ciphertextArry = QByteArray::fromBase64(ciphertextArry);
    int nCiphertextLen = ciphertextArry.length();
    uchar *pCiphertextData = reinterpret_cast<uchar *>(ciphertextArry.data());
    int nSize = RSA_public_decrypt(nCiphertextLen,
                                   pCiphertextData,
                                   reinterpret_cast<uchar *>(passwordBuf),
                                   pRsa,
                                   RSA_PKCS1_PADDING);
    QString qstrPassword = "";
    if (nSize >= 0) {
        qstrPassword = QByteArray(passwordBuf, nSize);
    }

    // 销毁对象
    delete[] passwordBuf;
    BIO_free_all(pPublicKeyBio);
    RSA_free(pRsa);

    return qstrPassword;
}
