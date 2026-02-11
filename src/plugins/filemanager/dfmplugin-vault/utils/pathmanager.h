// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PATHMANAGER_H
#define PATHMANAGER_H
#include "dfmplugin_vault_global.h"

#include <QObject>
namespace dfmplugin_vault {
class PathManager : public QObject
{
    Q_OBJECT
public:
    explicit PathManager(QObject *parent = nullptr);

public:
    static QString vaultLockPath();

    static QString vaultUnlockPath();

    static QString makeVaultLocalPath(const QString &path = "", const QString &base = "");

    static QString addPathSlash(const QString &path);
signals:

public slots:
};
}
#endif   // PATHMANAGER_H
