/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#ifndef USERSHAREDAEMONMANAGER_H
#define USERSHAREDAEMONMANAGER_H

#include <QDBusContext>
#include <QObject>

class UserShareAdaptor;

class UserShareManager : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    explicit UserShareManager(QObject *parent = nullptr);
    ~UserShareManager();

public:
    static QString ObjectPath;
    static QString PolicyKitActionId;

protected:
    bool checkAuthentication();

signals:

public slots:
    bool addGroup(const QString &groupName);
    bool setUserSharePassword(const QString &username, const QString &passward);
    bool closeSmbShareByShareName(const QString &sharename,const bool bshow);
    bool createShareLinkFile();

private:
    UserShareAdaptor* m_userShareAdaptor = nullptr;
};

#endif // USERSHAREDAEMONMANAGER_H
