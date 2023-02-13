// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RSAM_H
#define RSAM_H

#include <QString>

inline constexpr int kKeyLength { 2048 };   // 密钥的长度
inline constexpr char kKeyBegin[] { "BEGIN RSA PUBLIC KEY" };   // 代码生成key

class rsam
{
public:
    /*!
     * /brief createPublicAndPrivateKey 生成公钥和私钥对
     * /param publicKey 公钥
     * /param privateKey 私钥
     * /return 是否成功
     */
    static bool createPublicAndPrivateKey(QString &publicKey, QString &privateKey);

    /*!
     * /brief privateKeyEncrypt 私钥加密密码，生成密文
     * /param password 密码
     * /param privateKey 私钥
     * /return 密文
     */
    static QString privateKeyEncrypt(const QString &password, const QString &privateKey);

    /*!
     * /brief publicKeyDecrypt 公钥解密密文，获得密码
     * /param ciphertext 密文
     * /param strPubKey 公钥
     * /return 密码
     */
    static QString publicKeyDecrypt(const QString &ciphertext, const QString &publicKey);
};

#endif   // RSAM_H
