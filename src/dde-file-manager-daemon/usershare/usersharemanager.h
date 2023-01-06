// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USERSHAREDAEMONMANAGER_H
#define USERSHAREDAEMONMANAGER_H

#include <QDBusContext>
#include <QObject>

class UserShareAdaptor;

class UserShareManager : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface","com.deepin.filemanager.daemon.UserShareManager")

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
    bool isUserSharePasswordSet(const QString &username);

private:
    UserShareAdaptor* m_userShareAdaptor = nullptr;
};

#endif // USERSHAREDAEMONMANAGER_H
