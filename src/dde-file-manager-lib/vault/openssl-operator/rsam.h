/*
 * Copyright (C) 2016 ~ 2021 Uniontech Software Technology Co., Ltd.
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
*/

#ifndef RSAM_H
#define RSAM_H

#include <QString>

#define KEY_LENGTH              1024  // 密钥的长度
#define KEY_BEGIN               "BEGIN RSA PUBLIC KEY" // 代码生成key

class rsam
{
public:
    /**
     * @brief createPublicAndPrivateKey 生成公钥和私钥对
     * @param publicKey 公钥
     * @param privateKey 私钥
     * @return 是否成功
     */
    static bool createPublicAndPrivateKey(QString &publicKey, QString &privateKey);

    /**
     * @brief privateKeyEncrypt 私钥加密密码，生成密文
     * @param password 密码
     * @param privateKey 私钥
     * @return 密文
     */
    static QString privateKeyEncrypt(const QString &password, const QString &privateKey);

    /**
     * @brief publicKeyDecrypt 公钥解密密文，获得密码
     * @param ciphertext 密文
     * @param strPubKey 公钥
     * @return 密码
     */
    static QString publicKeyDecrypt(const QString &ciphertext, const QString &publicKey);
};




#endif // RSAM_H
