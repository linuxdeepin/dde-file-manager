/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
