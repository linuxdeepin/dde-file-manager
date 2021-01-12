/**
 * Copyright (C) 2020 Union Technology Co., Ltd.
 *
 * Author:     gong heng <gongheng@uniontech.com>
 *
 * Maintainer: gong heng <gongheng@uniontech.com>
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
 **/

#include "rsam.h"

#include <openssl/rsam.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <QDebug>

bool rsam::createRsaKey(QString &strPubKey, QString &strPriKey)

{
//    RSA *pRsa = RSA_generate_key(KEY_LENGTH, RSA_3, nullptr, nullptr);
//    if(!pRsa){
//        return false;
//    }
    RSA *pRsa = RSA_new();
    int ret = 0;
    BIGNUM *bne = BN_new();
    BN_set_word(bne, RSA_F4);
    ret = RSA_generate_key_ex(pRsa, KEY_LENGTH, bne, nullptr);
    if (ret != 1) {
        return false;
    }

    BIO *pPriBio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPrivateKey(pPriBio, pRsa, nullptr, nullptr, 0, nullptr, nullptr);

    BIO *pPubBio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPublicKey(pPubBio, pRsa);

    // 获取长度
    int nPriKeyLen = BIO_pending(pPriBio);
    int nPubKeyLen = BIO_pending(pPubBio);

    // 密钥对读取到字符串
    char *pPriKey = new char[nPriKeyLen];
    char *pPubKey = new char[nPubKeyLen];
    BIO_read(pPriBio, pPriKey, nPriKeyLen);
    BIO_read(pPubBio, pPubKey, nPubKeyLen);

    // 存储密钥对
    strPubKey = QByteArray(pPubKey, nPubKeyLen);
    strPriKey = QByteArray(pPriKey, nPriKeyLen);

    // 内存释放
    RSA_free(pRsa);
    BIO_free_all(pPriBio);
    BIO_free_all(pPubBio);
    delete[] pPriKey;
    delete[] pPubKey;

    return true;
}

// 私钥加密
QString rsam::rsa_pri_encrypt_base64(const QString &strClearData, const QString &strPriKey)
{
    QByteArray priKeyArry = strPriKey.toUtf8();
    uchar *pPriKey = reinterpret_cast<uchar *>(priKeyArry.data());
    BIO *pKeyBio = BIO_new_mem_buf(pPriKey, strPriKey.length());
    if (pKeyBio == nullptr) {
        return "";
    }

    RSA *pRsa = RSA_new();
    pRsa = PEM_read_bio_RSAPrivateKey(pKeyBio, &pRsa, nullptr, nullptr);
    if (pRsa == nullptr) {
        BIO_free_all(pKeyBio);
        return "";
    }

    int nLen = RSA_size(pRsa);
    char *pEncryptBuf = new char[nLen];
    memset(pEncryptBuf, 0, size_t(nLen));
    QByteArray clearDataArry = strClearData.toUtf8();
    int nClearDataLen = clearDataArry.length();
    uchar *pClearData = reinterpret_cast<uchar *>(clearDataArry.data());
    int nSize = RSA_private_encrypt(nClearDataLen,
                                    pClearData,
                                    reinterpret_cast<uchar *>(pEncryptBuf),
                                    pRsa,
                                    RSA_PKCS1_PADDING);

    QString strEncryptData = "";
    if (nSize >= 0) {
        QByteArray arry(pEncryptBuf, nSize);
        strEncryptData = arry.toBase64();
    }

    // 释放内存
    delete[] pEncryptBuf;
    BIO_free_all(pKeyBio);
    RSA_free(pRsa);

    return strEncryptData;
}




// 公钥解密
QString rsam::rsa_pub_decrypt_base64(const QString &strDecryptData, const QString &strPubKey)
{
    QByteArray pubKeyArry = strPubKey.toUtf8();
    uchar *pPubKey = reinterpret_cast<uchar *>(pubKeyArry.data());
    BIO *pKeyBio = BIO_new_mem_buf(pPubKey, strPubKey.length());
    if (pKeyBio == nullptr) {
        return "";
    }

    RSA *pRsa = RSA_new();
    if (strPubKey.contains(BEGIN_RSA_PUBLIC_KEY)) {
        pRsa = PEM_read_bio_RSAPublicKey(pKeyBio, &pRsa, nullptr, nullptr);
    } else {
        pRsa = PEM_read_bio_RSA_PUBKEY(pKeyBio, &pRsa, nullptr, nullptr);
    }

    if (!pRsa)
        return "";

    int nLen = RSA_size(pRsa);
    char *pClearBuf = new char[nLen];
    memset(pClearBuf, 0, size_t(nLen));

    // 解密
    QByteArray decryptDataArry = strDecryptData.toUtf8();
    decryptDataArry = QByteArray::fromBase64(decryptDataArry);
    int nDecryptDataLen = decryptDataArry.length();
    uchar *pDecryptData = reinterpret_cast<uchar *>(decryptDataArry.data());
    int nSize = RSA_public_decrypt(nDecryptDataLen,
                                   pDecryptData,
                                   reinterpret_cast<uchar *>(pClearBuf),
                                   pRsa,
                                   RSA_PKCS1_PADDING);
    QString strClearData = "";
    if (nSize >= 0) {
        strClearData = QByteArray(pClearBuf, nSize);
    }

    // 释放内存
    delete[] pClearBuf;
    BIO_free_all(pKeyBio);
    RSA_free(pRsa);

    return strClearData;
}

// 公钥加密
QString rsam::rsa_pub_encrypt_base64(const QString &strClearData, const QString &strPubKey)
{
    QByteArray pubKeyArry = strPubKey.toUtf8();
    uchar *pPubKey = reinterpret_cast<uchar *>(pubKeyArry.data());
    BIO *pKeyBio = BIO_new_mem_buf(pPubKey, pubKeyArry.length());
    if (pKeyBio == nullptr) {
        return "";
    }

    RSA *pRsa = RSA_new();
    if (strPubKey.contains(BEGIN_RSA_PUBLIC_KEY)) {
        pRsa = PEM_read_bio_RSAPublicKey(pKeyBio, &pRsa, nullptr, nullptr);
    } else {
        pRsa = PEM_read_bio_RSA_PUBKEY(pKeyBio, &pRsa, nullptr, nullptr);
    }
    if (pRsa == nullptr) {
        BIO_free_all(pKeyBio);
        return "";
    }

    int nLen = RSA_size(pRsa);
    char *pEncryptBuf = new char[nLen];
    memset(pEncryptBuf, 0, size_t(nLen));

    QByteArray clearDataArry = strClearData.toUtf8();
    int nClearDataLen = clearDataArry.length();
    uchar *pClearData = reinterpret_cast<uchar *>(clearDataArry.data());
    int nSize = RSA_public_encrypt(nClearDataLen,
                                   pClearData,
                                   reinterpret_cast<uchar *>(pEncryptBuf),
                                   pRsa,
                                   RSA_PKCS1_PADDING);
    QString strEncryptData = "";
    if (nSize >= 0) {
        QByteArray arry(pEncryptBuf, nSize);
        strEncryptData = arry.toBase64();
    }

    // 释放内存
    delete[] pEncryptBuf;
    BIO_free_all(pKeyBio);
    RSA_free(pRsa);
    return strEncryptData;
}




// 私钥解密
QString rsam::rsa_pri_decrypt_base64(const QString &strDecryptData, const QString &strPriKey)
{
    QByteArray priKeyArry = strPriKey.toUtf8();
    uchar *pPriKey = reinterpret_cast<uchar *>(priKeyArry.data());
    BIO *pKeyBio = BIO_new_mem_buf(pPriKey, priKeyArry.length());
    if (pKeyBio == nullptr) {
        return "";
    }

    RSA *pRsa = RSA_new();
    pRsa = PEM_read_bio_RSAPrivateKey(pKeyBio, &pRsa, nullptr, nullptr);
    if (pRsa == nullptr) {
        BIO_free_all(pKeyBio);
        return "";
    }
    int nLen = RSA_size(pRsa);
    char *pClearBuf = new char[nLen];
    memset(pClearBuf, 0, size_t(nLen));
    // 解密
    QByteArray decryptDataArry = strDecryptData.toUtf8();
    decryptDataArry = QByteArray::fromBase64(decryptDataArry);
    int nDecryptDataLen = decryptDataArry.length();
    uchar *pDecryptData = reinterpret_cast<uchar *>(decryptDataArry.data());
    int nSize = RSA_private_decrypt(nDecryptDataLen,
                                    pDecryptData,
                                    reinterpret_cast<uchar *>(pClearBuf),
                                    pRsa,
                                    RSA_PKCS1_PADDING);
    QString strClearData = "";
    if (nSize >= 0) {
        strClearData = QByteArray(pClearBuf, nSize);
    }

    // 释放内存
    delete[] pClearBuf;
    BIO_free_all(pKeyBio);
    RSA_free(pRsa);

    return strClearData;

}

void rsam::test()
{
    /**
     * rsa private/public key 若从文件中拷贝出来，
     * 需要注意保存原先文件格式，及换行符需要带上，
     * 包括最后一行的换行符
     */
    QString strPriKey = "";
    QString strPubKey = "";
    /**
     * 用代码生成的key与openssl命令生成的key区别：
     * 1. 代码生成key，标题为 ----BEGIN RSA PUBLIC KEY----，
     * openssl命令生成key，标题为 ----BEGIN PUBLIC KEY----
     * 2. 获取RSA函数不同，代码生成key，用PEM_read_bio_RSAPublicKey,
     * openssl命令生成key，用PEM_read_bio_RSA_PUBKEY
     */
    createRsaKey(strPubKey, strPriKey);

    QString strEncryptData;
    QString strClearData;
    QString strClear = "hello";

    qDebug() << "password:\n" << strClear;
    qDebug() << "public key:\n" << strPubKey;
    qDebug() << "private key:\n" << strPriKey;
    qDebug() << endl;

    qDebug() << "private key encrypt, public key decrypt";
    qDebug() << "---------------------------------------";
    strEncryptData = rsa_pri_encrypt_base64(strClear, strPriKey);
    qDebug() << "encrypt data:\n" << strEncryptData;
    strClearData = rsa_pub_decrypt_base64(strEncryptData, strPubKey);
    qDebug() << "decrypt data:\n" << strClearData;
    qDebug() << endl;

    qDebug() << "public key encrypt, private key decrypt";
    qDebug() << "---------------------------------------";
    strEncryptData = rsa_pub_encrypt_base64(strClear, strPubKey);
    qDebug() << "encrypt data:\n" << strEncryptData;
    strClearData = rsa_pri_decrypt_base64(strEncryptData, strPriKey);
    qDebug() << "decrypt data:\n" << strClearData;
    qDebug() << endl;

    qDebug() << "public key encrypt, private key decrypt";
    qDebug() << "---------------------------------------";
    strEncryptData = rsa_pub_encrypt_base64(strClear, strPubKey);
    qDebug() << "encrypt data:\n" << strEncryptData;
    strClearData = rsa_pri_decrypt_base64(strEncryptData, strPriKey);
    qDebug() << "decrypt data:\n" << strClearData;
    qDebug() << endl;
}
