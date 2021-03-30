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

#include "pbkdf2.h"

#include <QString>
#include <openssl/bn.h>
#include <QDebug>
#include <openssl/evp.h>

char pbkdf2::charToHexadecimalChar(char nibble)
{
    char buf[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
                   };
    return buf[nibble & 0xF];
}

char *pbkdf2::octalToHexadecimal(const char *str, int length)
{
    const char *s = str;
    int i = 0;
    length *= 2;

    if (length > CIPHER_LENGHT_MAX)
        length = CIPHER_LENGHT_MAX - 1;

    char *bit_string = reinterpret_cast<char *>(malloc(size_t(length + 1)));

    for (i = 0; i < length; i += 2) {
        bit_string[i] = charToHexadecimalChar(*s >> 4);
        bit_string[i + 1] = charToHexadecimalChar(*s & 0xF);
        s++;
    }
    bit_string[i] = 0;

    return bit_string;
}

// 生成随机盐
QString pbkdf2::createRandomSalt(int byte)

{
    BIGNUM *rnd = BN_new();
    int bits = byte * 4;
    int top = 0, bottom = 0;
    BN_rand(rnd, bits, top, bottom);

    char *cstr = BN_bn2hex(rnd);
    QString strRandSalt = QString::fromUtf8(QByteArray(cstr));
    BN_free(rnd);

    return strRandSalt;

}

QString pbkdf2::pbkdf2EncrypyPassword(const QString &password, const QString &randSalt, int iteration, int cipherByteNum)
{
    if (cipherByteNum < 0 || cipherByteNum % 2 != 0) {
        qDebug() << "cipherByteNum can't less than zero and must be even!";
        return "";
    }
    // 字节长度
    int nCipherLength = cipherByteNum / 2;

    // 格式化随机盐
    uchar salt_value[SALT_LENGTH_MAX];
    memset(salt_value, 0, SALT_LENGTH_MAX);
    int nSaltLength = randSalt.length();
    for (int i = 0; i < nSaltLength; i++) {
        salt_value[i] = uchar(randSalt.at(i).toLatin1());
    }

    // 生成密文
    QString strCipherText("");
    uchar *out = reinterpret_cast<uchar *>(malloc(size_t(cipherByteNum / 2 + 1)));
    memset(out, 0, size_t(cipherByteNum / 2 + 1));
    // 修复bug-51478
    // 修复bug-60724
    std::string strPassword = password.toStdString();
    const char *pwd = strPassword.c_str();
    if (PKCS5_PBKDF2_HMAC_SHA1(pwd, password.length(),
                               salt_value, randSalt.length(),
                               iteration,
                               nCipherLength,
                               out) != 0) {
        char *pstr = octalToHexadecimal(reinterpret_cast<char *>(out), nCipherLength);
        // 修复bug-51478
        strCipherText = QString(pstr);
        if (pstr)
            free(pstr);
    } else {
        qDebug() << "PKCS5_PBKDF2_HMAC_SHA1 failed";
    }
    free(out);
    return strCipherText;
}
