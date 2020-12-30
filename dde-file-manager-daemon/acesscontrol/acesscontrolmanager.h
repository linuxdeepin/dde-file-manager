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

#ifndef ACESSCONTROLMANAGER_H
#define ACESSCONTROLMANAGER_H

#include <QDBusContext>
#include <QObject>

class AcessControlAdaptor;
class DDiskManager;
class DFileSystemWatcher;

class AcessControlManager : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface","com.deepin.filemanager.daemon.AcessControlManager")

public:
    explicit AcessControlManager(QObject *parent = nullptr);
    ~AcessControlManager();

    void initConnect();

    static QString ObjectPath;
    static QString PolicyKitActionId;

protected:
    bool checkAuthentication();

private slots:
    void onFileCreated(const QString &path, const QString &name);
    void chmodMountpoints(const QString &blockDevicePath, const QByteArray &mountPoint);

private:
    AcessControlAdaptor *m_acessControlAdaptor = nullptr;
    DDiskManager *m_diskMnanager = nullptr;
    DFileSystemWatcher *m_watcher = nullptr;
};

#endif // ACESSCONTROLMANAGER_H
