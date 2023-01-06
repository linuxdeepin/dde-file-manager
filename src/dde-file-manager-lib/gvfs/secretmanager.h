// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SECRETMANAGER_H
#define SECRETMANAGER_H

#include <durl.h>
#include <QObject>
#include <QJsonObject>
#include <QMutex>

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
    void clearPassworkBySmbHost(const DUrl &smbDevice);
    bool userCheckedRememberPassword(const DUrl &smbDevice);
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
    QMutex smbMutex;
};

#endif // SECRETMANAGER_H
