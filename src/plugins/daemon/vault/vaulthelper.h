// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTHELPER_H
#define VAULTHELPER_H

#include "daemonplugin_vaultdaemon_global.h"

#include <QObject>

DAEMONPVAULT_BEGIN_NAMESPACE
class VaultHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(VaultHelper)
public:
    static VaultHelper *instance();

    inline QString scheme()
    {
        return "dfmvault";
    }

    bool isVaultFile(const QUrl &url);
    QString vaultBaseDirLocalPath();
    QString vaultMountDirLocalPath();
    QString buildVaultLocalPath(const QString &path = "", const QString &base = "");
    QUrl vaultUrlToLocalUrl(const QUrl &url);
    QList<QUrl> transUrlsToLocal(const QList<QUrl> &urls);

private:
    explicit VaultHelper(QObject *parent = nullptr);
};
DAEMONPVAULT_END_NAMESPACE

#endif // VAULTHELPER_H
