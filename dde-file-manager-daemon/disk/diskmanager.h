/*
* Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
*
* Author: Liu Zhangjian<liuzhangjian@uniontech.com>
*
* Maintainer: Liu Zhangjian<liuzhangjian@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <QObject>
#include <QDBusContext>
#include <QDBusArgument>

class DiskAdaptor;

struct crypt_device;

class DiskManager : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    explicit DiskManager(QObject *parent = nullptr);
    ~DiskManager();

    static QString ObjectPath;
    static QString PolicyKitActionId;

protected:
    bool checkAuthentication();
    int changePassword(const char* oldPwd, const char *newPwd, const char *device);

signals:
    void confirmed(bool state);
    void finished(int code);

public slots:
    void changeDiskPassword(const QString &oldPwd, const QString &newPwd);

private:
    DiskAdaptor *m_diskAdaptor = nullptr;
};

#endif // DISKMANAGER_H
