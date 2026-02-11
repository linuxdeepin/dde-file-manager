// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pbkdf2.h"

#include <QString>
#include <openssl/bn.h>
#include <QDebug>
#include <openssl/evp.h>

DPVAULT_USE_NAMESPACE

char pbkdf2::charToHexadecimalChar(char nibble)
{
    char buf[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                     '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    return buf[nibble & 0xF];
}

char *pbkdf2::octalToHexadecimal(const char *str, int length)
{
    fmDebug() << "Vault: Converting octal to hexadecimal, length:" << length;

    const char *s = str;
    int i = 0;
    length *= 2;

    if (length > CIPHER_LENGHT_MAX) {
        fmWarning() << "Vault: Length exceeds maximum, truncating to:" << (CIPHER_LENGHT_MAX - 1);
        length = CIPHER_LENGHT_MAX - 1;
    }

    char *bit_string = reinterpret_cast<char *>(malloc(size_t(length + 1)));

    for (i = 0; i < length; i += 2) {
        bit_string[i] = charToHexadecimalChar(*s >> 4);
        bit_string[i + 1] = charToHexadecimalChar(*s & 0xF);
        s++;
    }
    bit_string[i] = 0;

    fmDebug() << "Vault: Octal to hexadecimal conversion completed, result length:" << i;
    return bit_string;
}

// 生成随机盐
QString pbkdf2::createRandomSalt(int byte)
{
    fmDebug() << "Vault: Creating random salt with byte length:" << byte;

    BIGNUM *rnd = BN_new();
    int bits = byte * 4;
    int top = 0, bottom = 0;
    BN_rand(rnd, bits, top, bottom);

    char *cstr = BN_bn2hex(rnd);
    QString strRandSalt = QString::fromUtf8(QByteArray(cstr));
    free(cstr);
    BN_free(rnd);

    fmDebug() << "Vault: Random salt created successfully, length:" << strRandSalt.length();
    return strRandSalt;
}

QString pbkdf2::pbkdf2EncrypyPassword(const QString &password, const QString &randSalt, int iteration, int cipherByteNum)
{
    if (cipherByteNum < 0 || cipherByteNum % 2 != 0) {
        fmCritical() << "Vault: cipherByteNum can't less than zero and must be even!";
        return "";
    }
    // 字节长度
    int nCipherLength = cipherByteNum / 2;
    fmDebug() << "Vault: Cipher length in bytes:" << nCipherLength;

    // 格式化随机盐
    uchar salt_value[SALT_LENGTH_MAX];
    memset(salt_value, 0, SALT_LENGTH_MAX);
    int nSaltLength = randSalt.length();
    for (int i = 0; i < nSaltLength; i++) {
        salt_value[i] = uchar(randSalt.at(i).toLatin1());
    }
    fmDebug() << "Vault: Salt formatted, length:" << nSaltLength;

    // 生成密文
    QString strCipherText("");
    uchar *out = reinterpret_cast<uchar *>(malloc(size_t(cipherByteNum / 2 + 1)));
    memset(out, 0, size_t(cipherByteNum / 2 + 1));
    // 修复bug-51478
    // 修复bug-60724
    std::string strPassword = password.toStdString();
    const char *pwd = strPassword.c_str();

    fmDebug() << "Vault: Calling PKCS5_PBKDF2_HMAC_SHA1";
    if (PKCS5_PBKDF2_HMAC_SHA1(pwd, password.length(),
                               salt_value, randSalt.length(),
                               iteration,
                               nCipherLength,
                               out)
        != 0) {
        fmDebug() << "Vault: PKCS5_PBKDF2_HMAC_SHA1 succeeded";
        char *pstr = octalToHexadecimal(reinterpret_cast<char *>(out), nCipherLength);
        // 修复bug-51478
        strCipherText = QString(pstr);
        if (pstr)
            free(pstr);
    } else {
        fmCritical() << "Vault: the function of PKCS5_PBKDF2_HMAC_SHA1 failed";
    }

    free(out);
    fmDebug() << "Vault: PBKDF2 encryption completed";
    return strCipherText;
}
