#ifndef PBKDF2_H
#define PBKDF2_H

#define SALT_LENGTH_MAX         100
#define CLIPHER_LENGHT_MAX      100

class QString;

class pbkdf2
{
public:
    /**
     * @brief createRandomSalt 创建随机数（十六进制字符串）
     * @param byte 字符串的字节数
     * @return 随机字符串
     */
    static QString createRandomSalt(int byte);

    /**
     * @brief pbkdf2EncrypyPassword PBKDF2加密密码
     * @param password 密码
     * @param randSalt 随机盐
     * @param iteration 迭代次数
     * @param clipherByteNum 生成密文的字节数
     * @return 加密后的密文
     */
    static QString pbkdf2EncrypyPassword(const QString &password,

                                         const QString &randSalt,

                                         int iteration,

                                         int clipherByteNum);

private:
    static char nibble_to_hex_char(char nibble);
    static char *octet_string_hex_string(const char *str, int length);

};




#endif // PBKDF2_H
