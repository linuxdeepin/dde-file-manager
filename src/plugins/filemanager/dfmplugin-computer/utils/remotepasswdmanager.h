// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REMOTEPASSWDMANAGER_H
#define REMOTEPASSWDMANAGER_H

#include "dfmplugin_computer_global.h"

#include <QObject>
#include <QJsonObject>
#include <QMutex>

#undef signals
extern "C" {
#include <libsecret/secret.h>
}
#define signals public

#define RemotePasswdManagerInstance DPCOMPUTER_NAMESPACE::RemotePasswdManager::instance()

namespace dfmplugin_computer {

class RemotePasswdManager : public QObject
{
    Q_OBJECT

public:
    static RemotePasswdManager *instance();
    void clearPasswd(const QString &uri);   // smb://1.2.3.4/sharefolder

private:
    explicit RemotePasswdManager(QObject *parent = nullptr);

    static const SecretSchema *smbSchema();
    static const SecretSchema *ftpSchema();

    static void onPasswdCleared(GObject *obj, GAsyncResult *res, gpointer data);

private:
    QJsonObject smbObjs;
    QMutex mutex;
};

}

#endif   // REMOTEPASSWDMANAGER_H
