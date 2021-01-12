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

#ifndef RSA_H
#define RSA_H

#include <QString>

// define rsa public key
#define BEGIN_RSA_PUBLIC_KEY    "BEGIN RSA PUBLIC KEY" // 代码生成key
#define BEGIN_PUBLIC_KEY        "BEGIN PUBLIC KEY" // 命令生成key
#define KEY_LENGTH              1024  // 密钥长度

class rsam
{
public:
    /**
     * @brief createRsaKey 生成密钥对
     * @param strPubKey 公钥
     * @param strPriKey 私钥
     * @return 状态
     */
    static bool createRsaKey(QString &strPubKey, QString &strPriKey);

    /**
     * @brief rsa_pri_encrypt_base64 私钥加密
     * @param strClearData 明文
     * @param strPriKey 私钥
     * @return 加密后数据（base64格式）
     */
    static QString rsa_pri_encrypt_base64(const QString &strClearData, const QString &strPriKey);

    /**
     * @brief rsa_pub_decrypt_base64 公钥解密
     * @param strDecryptData 代解密数据（baee64格式）
     * @param strPubKey 公钥
     * @return 明文
     */
    static QString rsa_pub_decrypt_base64(const QString &strDecryptData, const QString &strPubKey);

    /**
     * @brief rsa_pub_encrypt_base64 公钥加密
     * @param strClearData 明文
     * @param strPubKey 公钥
     * @return 加密后数据（base64）
     */
    static QString rsa_pub_encrypt_base64(const QString &strClearData, const QString &strPubKey);

    /**
     * @brief rsa_pri_decrypt_base64 私钥解密
     * @param strClearData 待解密数据（base64格式）
     * @param strPriKey 私钥
     * @return 明文
     */
    static QString rsa_pri_decrypt_base64(const QString &strDecryptData, const QString &strPriKey);

    /**
     * @brief test 测试
     */
    static void test();
};




#endif // RSA_H
