/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef SECRETMANAGER_H
#define SECRETMANAGER_H

#include <QObject>
#include <QJsonObject>

#include <DSecureString>

DCORE_USE_NAMESPACE

#undef signals
extern "C" {
    #include <libsecret/secret.h>
}
#define signals public

class SecretManager : public QObject
{
    Q_OBJECT
public:
    explicit SecretManager(QObject *parent = nullptr);
    ~SecretManager();

    void initData();
    void initConnect();

    static const SecretSchema * SMBSecretSchema();
    static const SecretSchema * FTPSecretSchema();
    static const SecretSchema * VaultSecretSchema();

    static void on_password_cleared (GObject *source,
                                GAsyncResult *result,
                                gpointer unused);

    bool storeVaultPassword(const DSecureString &string);
    DSecureString lookupVaultPassword();
    bool clearVaultPassword();
    void clearPasswordByLoginObj(const QJsonObject& obj);
    QJsonObject getLoginData(const QString& id);
    QJsonObject getLoginDatas();
    static QString cachePath();
signals:

public slots:
    void cacheSambaLoginData(const QJsonObject& obj);
    void loadCache();
    void saveCache();


private:
    QJsonObject m_smbLoginObjs;
};

#endif // SECRETMANAGER_H
