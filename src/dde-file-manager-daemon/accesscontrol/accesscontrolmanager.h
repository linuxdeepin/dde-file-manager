/*
 * Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
 *
 * Author:     zhangsheng <zhangsheng@uniontech.com>
 *
 * Maintainer: zhangsheng <zhangsheng@uniontech.com>
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

#ifndef ACCESSCONTROLMANAGER_H
#define ACCESSCONTROLMANAGER_H

#include <QDBusContext>
#include <QObject>

class AccessControlAdaptor;
class DDiskManager;
class DFileSystemWatcher;

class AccessControlManager : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface","com.deepin.filemanager.daemon.AccessControlManager")

public:
    explicit AccessControlManager(QObject *parent = nullptr);
    ~AccessControlManager();

    void initConnect();

    static QString ObjectPath;
    static QString PolicyKitActionId;

protected:
    bool checkAuthentication();

private slots:
    void onFileCreated(const QString &path, const QString &name);
    void chmodMountpoints(const QString &blockDevicePath, const QByteArray &mountPoint);

private:
    AccessControlAdaptor *m_accessControlAdaptor = nullptr;
    DDiskManager *m_diskMnanager = nullptr;
    DFileSystemWatcher *m_watcher = nullptr;
};

#endif // ACCESSCONTROLMANAGER_H
