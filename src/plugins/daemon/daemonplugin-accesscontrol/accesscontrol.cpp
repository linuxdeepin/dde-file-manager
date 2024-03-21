// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "accesscontrol.h"
#include "dbusadaptor/accesscontrolmanager_adaptor.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef COMPILE_ON_V23
#    define ACCOUNTS_BUS_NAME "org.deepin.dde.Accounts1"
#    define ACCOUNTS_OBJ_PATH "/org/deepin/dde/Accounts1"
#    define ACCOUNTS_INTERFACE "org.deepin.dde.Accounts1"
#    define ACCOUNTS_USER_INTERFACE "org.deepin.dde.Accounts1.User"
#else
#    define ACCOUNTS_BUS_NAME "com.deepin.daemon.Accounts"
#    define ACCOUNTS_OBJ_PATH "/com/deepin/daemon/Accounts"
#    define ACCOUNTS_INTERFACE "com.deepin.daemon.Accounts"
#    define ACCOUNTS_USER_INTERFACE "com.deepin.daemon.Accounts.User"
#endif

namespace daemonplugin_accesscontrol {

DFM_LOG_REISGER_CATEGORY(DAEMONPAC_NAMESPACE)

bool AccessControl::start()
{
    if (!isDaemonServiceRegistered())
        return false;

    createUserMountDirs();
    initConnect();
    initDBusInterce();
    return true;
}

bool AccessControl::isDaemonServiceRegistered()
{
    static constexpr char kEnvNameOfDaemonRegistered[] { "DAEMON_SERVICE_REGISTERED" };

    QByteArray registered { qgetenv(kEnvNameOfDaemonRegistered) };
    fmInfo() << "Env DAEMON_SERVICE_REGISTERED is: " << registered;
    if (QString::fromLocal8Bit(registered) == "TRUE")
        return true;

    return false;
}

void AccessControl::initDBusInterce()
{
    accessControlManager.reset(new AccessControlDBus);
    Q_UNUSED(new AccessControlManagerAdaptor(accessControlManager.data()));
    if (!QDBusConnection::systemBus().registerObject("/com/deepin/filemanager/daemon/AccessControlManager", accessControlManager.data())) {
        fmWarning("Cannot register the \"/com/deepin/filemanager/daemon/AccessControlManager\" object.\n");
        accessControlManager.reset(nullptr);
        return;
    }
}

void AccessControl::initConnect()
{
    QDBusConnection::systemBus().connect(ACCOUNTS_BUS_NAME,
                                         ACCOUNTS_OBJ_PATH,
                                         ACCOUNTS_INTERFACE,
                                         "UserAdded",
                                         this,
                                         SLOT(createUserMountDir(const QString &)));
}

void AccessControl::createUserMountDir(const QString &objPath)
{
    QDBusInterface userIface(ACCOUNTS_BUS_NAME,
                             objPath,
                             ACCOUNTS_USER_INTERFACE,
                             QDBusConnection::systemBus());
    const QString &userName = userIface.property("UserName").toString();
    fmInfo() << "about to create mount dir of user" << userName << objPath;

    const QString &userMountRoot = QString("/media/%1").arg(userName);
    if (!QDir(userMountRoot).exists()) {
        if (QDir().mkpath(userMountRoot)) {
            fmInfo() << userMountRoot << "created.";
            struct stat fileStat;
            QByteArray nameBytes(userMountRoot.toUtf8());
            stat(nameBytes.data(), &fileStat);
            chmod(nameBytes.data(), (fileStat.st_mode | S_IRUSR | S_IRGRP | S_IROTH));
        }
    }
    // ACL
    QString aclCmd = QString("setfacl -m o:rx %1").arg(userMountRoot);
    QProcess::execute(aclCmd);
    fmInfo() << "acl the /media/anyuser folder";
}

void AccessControl::createUserMountDirs()
{
    // get user list by com.deepin.daemon.Accounts
    QDBusInterface iface(ACCOUNTS_BUS_NAME,
                         ACCOUNTS_OBJ_PATH,
                         ACCOUNTS_INTERFACE,
                         QDBusConnection::systemBus());
    QStringList userNames;
    const QStringList &userList = iface.property("UserList").toStringList();
    for (const auto &objPath : userList)
        createUserMountDir(objPath);
}

}   // namespace daemonplugin_accesscontrol
