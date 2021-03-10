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

#ifndef PBKDF2_H
#define PBKDF2_H

#define SALT_LENGTH_MAX         100
#define CIPHER_LENGHT_MAX       100

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
     * @param cipherByteNum 生成密文的字节数
     * @return 加密后的密文
     */
    static QString pbkdf2EncrypyPassword(const QString &password,

                                         const QString &randSalt,

                                         int iteration,

                                         int cipherByteNum);

private:
    static char charToHexadecimalChar(char nibble);
    static char *octalToHexadecimal(const char *str, int length);

};




#endif // PBKDF2_H
