/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
    void loadConfig();
    void saveConfig();
    QJsonObject getLoginInfo(const QString &uri);
    QString parseServer(const QString &uri);
    QString configPath();

    static const SecretSchema *smbSchema();
    static const SecretSchema *ftpSchema();

    static void onPasswdCleared(GObject *obj, GAsyncResult *res, gpointer data);

private:
    QJsonObject smbObjs;
    QMutex mutex;
};

}

#endif   // REMOTEPASSWDMANAGER_H
