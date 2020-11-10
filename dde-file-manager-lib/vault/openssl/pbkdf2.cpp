#include "pbkdf2.h"

#include <QString>
#include <openssl/bn.h>
#include <QDebug>
#include <openssl/evp.h>

char pbkdf2::nibble_to_hex_char(char nibble)
{
    char buf[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    return buf[nibble & 0xF];
}

char *pbkdf2::octet_string_hex_string(const char *str, int length)
{
    const char *s = str;
    int i = 0;
    length *= 2;

    if(length > CLIPHER_LENGHT_MAX)
        length = CLIPHER_LENGHT_MAX - 1;

    char *bit_string = reinterpret_cast<char *>(malloc(size_t(length+1)));

    for(i = 0; i < length; i += 2){
        bit_string[i] = nibble_to_hex_char(*s >> 4);
        bit_string[i+1] = nibble_to_hex_char(*s & 0xF);
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

QString pbkdf2::pbkdf2EncrypyPassword(const QString &password, const QString &randSalt, int iteration, int clipherByteNum)
{
    if(clipherByteNum < 0 || clipherByteNum % 2 != 0)
    {
        qDebug() << "clipherByteNum can't less than zero and must be even!";
        return "";
    }
    // 字节长度
    int nClipherLength = clipherByteNum / 2;

    // 格式化随机盐
    uchar salt_value[SALT_LENGTH_MAX];
    memset(salt_value, 0, SALT_LENGTH_MAX);
    int nSaltLength = randSalt.length();
    for(int i = 0; i < nSaltLength; i++){
        salt_value[i] = uchar(randSalt.at(i).toLatin1());
    }

    // 生成密文
    QString strClipherText("");
    uchar *out = reinterpret_cast<uchar*>(malloc(size_t(clipherByteNum/2 + 1)));
    memset(out, 0, size_t(clipherByteNum/2 + 1));
    // 修复wayland-bug-51478 QString转换成char*
    const char *pwd = password.toStdString().c_str();
    if(PKCS5_PBKDF2_HMAC_SHA1(pwd, password.length(),
                              salt_value, randSalt.length(),
                              iteration,
                              nClipherLength,
                              out) != 0){
        char *pstr = octet_string_hex_string(reinterpret_cast<char*>(out), nClipherLength);
        // 修复wayland-bug-51478 char*转换成QString
        strClipherText = QString(pstr);
    }else{
        qDebug() << "PKCS5_PBKDF2_HMAC_SHA1 failed";
    }
    free(out);
    return strClipherText;
}
