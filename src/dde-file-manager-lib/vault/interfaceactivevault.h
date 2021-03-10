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

#ifndef INTERFACEACTIVEVAULT_H
#define INTERFACEACTIVEVAULT_H

#include "vaultglobaldefine.h"

#include <QObject>

class InterfaceActiveVault : public QObject
{
    Q_OBJECT
public:
    explicit InterfaceActiveVault(QObject *parent = nullptr);
    ~InterfaceActiveVault();

    /**
     * @brief getPassword 获取密码
     * @return
     */
    static QString getPassword();

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
    static EN_VaultState vaultState();

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

#endif // INTERFACEACTIVEVAULT_H
