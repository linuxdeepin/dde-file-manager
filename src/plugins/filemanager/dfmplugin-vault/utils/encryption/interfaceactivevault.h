// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INTERFACEACTIVEVAULT_H
#define INTERFACEACTIVEVAULT_H

#include "dfmplugin_vault_global.h"
#include "utils/vaultdefine.h"

#include <QObject>

namespace dfmplugin_vault {
class InterfaceActiveVault : public QObject
{
    Q_OBJECT
public:
    explicit InterfaceActiveVault(QObject *parent = nullptr);
    ~InterfaceActiveVault();

    /**
     * @brief getPasswordHint 获得密码提示信息
     * @param passwordHint 返回密码提示
     * @return 是否成功
     */
    static bool getPasswordHint(QString &passwordHint);

    /**
     * @brief checkPassword 验证密码是否正确
     * @param password 密码
     * @param cipher 返回密码的密文
     * @return 是否成功
     */
    static bool checkPassword(const QString &password, QString &cipher);

    /**
     * @brief checkUserKey 验证密钥是否正确
     * @param userKey 密钥
     * @param cipher 返回密码的密文
     * @return 是否成功
     */
    static bool checkUserKey(const QString &userKey, QString &cipher);

    /**
     * @brief getEncryptedDir 获得加密文件夹路径
     * @return
     */
    static QString getEncryptDir();

    /**
     * @brief getDecryptDir 获得解密文件夹路径
     * @return
     */
    static QString getDecryptDir();

    /**
     * @brief getConfigFilePath
     * @return
     */
    static QStringList getConfigFilePath();

    /**
     * @brief vaultState // 获得保险箱状态
     * @return
     */
    static VaultState vaultState();

    /**
     * @brief getRootPassword 管理员权限认证
     * @return
     */
    static bool getRootPassword();

    /**
    * @brief executionShellCommand 执行shell命令并获得shell命令的返回值
    * @param strCmd 要执行的shell命令
    * @param lstShellOutput shell命令返回的结果
    * @return 返回值为0表示成功，其它都不成功
    */
    static int executionShellCommand(const QString &strCmd, QStringList &lstShellOutput);

signals:

public slots:

private:
};
}
#endif   // INTERFACEACTIVEVAULT_H
