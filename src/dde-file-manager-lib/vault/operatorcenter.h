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

#ifndef OPERATORCENTER_H
#define OPERATORCENTER_H

#include "vaultglobaldefine.h"

#include <DSecureString>

#include <QObject>

class OperatorCenter : public QObject
{
    Q_OBJECT
public:
    static OperatorCenter *getInstance();
    ~OperatorCenter();

    /**
     * @brief createDirAndFile 创建保险箱目录及密钥文件
     * @return 是否成功
     */
    bool createDirAndFile();

    /**
     * @brief saveSaltAndCiphertext pbkdf2加密密码，并将盐和密文写入密码文件
     * @param password 密码
     * @param passwordHint 密码提示
     * @return 是否成功
     */
    bool saveSaltAndCiphertext(const QString &password, const QString &passwordHint);

    /**
     * @brief createKey rsa生成密钥对，私钥加密密码，将密文写入文件，将一部分公钥写入文件（另一部分公钥提供给用户）
     * @param password 密码
     * @param bytes 提供给用户的那部分公钥的长度
     * @return 是否成功
     */
    bool createKey(const QString &password, int bytes);

    /**
     * @brief checkPassword 验证密码是否正确
     * @param password 密码
     * @param cipher 如果密码正确，返回加密后的密文，用于解锁保险箱
     * @return 是否正确
     */
    bool checkPassword(const QString &password, QString &cipher);

    /**
     * @brief checkUserKey 验证用户密钥是否正确
     * @param userKey 用户密钥
     * @param cipher 如果密钥正确，返回加密后的密文，用于解锁保险箱
     * @return 是否正确
     */
    bool checkUserKey(const QString &userKey, QString &cipher);

    /**
     * @brief getUserKey 获得用户密钥
     * @return 用户密钥
     */
    QString getUserKey();

    /**
     * @brief getPasswordHint 获取密码提示信息
     * @param passwordHint 返回提示信息
     * @return 是否成功
     */
    bool getPasswordHint(QString &passwordHint);

    /**
     * @brief createQRCode 生成二维码
     * @param srcStr 数据源
     * @param width 二维码图片宽度
     * @param height 二维码图片高度
     * @param pix   返回二维码图片
     * @return 是否成功
     */
    bool createQRCode(const QString &srcStr, int width, int height, QPixmap &pix);

    //! 获取保险箱状态
    EN_VaultState vaultState();

    //! 获取盐值及密码密文
    QString getSaltAndPasswordCipher();

    //! 清除盐值及密码密文的缓存
    void clearSaltAndPasswordCipher();

    //! 获得加密文件夹路径
    QString getEncryptDirPath();
    //! 获得解密文件夹路径
    QString getdecryptDirPath();

    //! 获得配置文件路径
    QStringList getConfigFilePath();

    //! 随即生成密码
    QString autoGeneratePassword(int minLength);

    //! 管理员权限认证
    bool getRootPassword();

    //! 执行shell命令并获得shell命令的返回值
    int executionShellCommand(const QString &strCmd, QStringList &lstShellOutput);

private:
    explicit OperatorCenter(QObject *parent = nullptr);
    // 组织保险箱本地文件路径
    QString makeVaultLocalPath(const QString &before = "", const QString &behind = "");
    bool runCmd(const QString &cmd);
    bool executeProcess(const QString &cmd);
    // 保存第二次加密后的密文,并更新保险箱版本信息
    bool secondSaveSaltAndCiphertext(const QString &ciphertext, const QString &salt, const char *vaultVersion);

public:
    /**
     * @brief createKeyNew rsa生成密钥对，私钥加密密码，将密文写入文件
     * @param in password 密码
     * @return 是否成功
     */
    bool createKeyNew(const QString &password);

    /**
     * @brief saveKey 保存公钥
     * @param key  公钥
     * @param path 保存路径
     * @return
     */
    bool saveKey(QString key, QString path);

    /**
     * @brief getPubKey 获得密码公钥
     * @return 密码公钥
     */
    QString getPubKey();

    /**
     * @brief verificationRetrievePassword 验证用户密钥是否正确
     * @param1 in keypath 密钥文件
     * @param2 out password 密码
     * @return 是否正确
     */
    bool verificationRetrievePassword(const QString keypath, QString &password);

private:
    Dtk::Core::DSecureString m_strCryfsPassword;   // cryfs密码
    QString m_strUserKey;
    QString m_standOutput;
    QString m_strPubKey;
};

#endif   // OPERATORCENTER_H
