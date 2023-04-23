// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sysinfoutils.h"
#include <dfm-base/dfm_global_defines.h>

#include <DSysInfo>

#include <QDBusInterface>
#include <QDBusReply>

#include <unistd.h>

DCORE_USE_NAMESPACE
using namespace dfmbase;

QString SysInfoUtils::getUser()
{
    static QString user = QString::fromLocal8Bit(qgetenv("USER"));

    return user;
}

QString SysInfoUtils::getHostName()
{
    static QString name;
    if (!name.isEmpty())
        return name;

#if (_XOPEN_SOURCE >= 500) || (_POSIX_C_SOURCE >= 200112L)
    char buf[256] { 0 };
    if (::gethostname(buf, sizeof(buf)) == 0) {
        name = QString(buf);
        return name;
    }
#endif

    return {};
}

int SysInfoUtils::getUserId()
{
    return static_cast<int>(getuid());
}

bool SysInfoUtils::isRootUser()
{
    return getUserId() == 0;
}

bool SysInfoUtils::isServerSys()
{
    return DSysInfo::deepinType() == DSysInfo::DeepinServer;
}

bool SysInfoUtils::isDesktopSys()
{
    return !(SysInfoUtils::isServerSys());
}

bool SysInfoUtils::isOpenAsAdmin()
{
    return SysInfoUtils::isRootUser() && SysInfoUtils::isDesktopSys();
}

bool SysInfoUtils::isDeveloperModeEnabled()
{
    // 为了性能，开发者模式仅需获取一次，不必每次请求 dbus，因此此处使用静态变量
    static bool hasAcquireDevMode = false;
    static bool developerModel = false;

    if (Q_UNLIKELY(!hasAcquireDevMode)) {
        hasAcquireDevMode = true;
        QString service("com.deepin.sync.Helper");
        QString path("/com/deepin/sync/Helper");
        QString interfaceName("com.deepin.sync.Helper");

        QDBusInterface interface(service, path, interfaceName, QDBusConnection::systemBus());

        QString func("IsDeveloperMode");
        QDBusReply<bool> reply = interface.call(func);
        developerModel = reply.value();
    }

    return developerModel;
}

bool SysInfoUtils::isProfessional()
{
    return DSysInfo::deepinType() == DSysInfo::DeepinProfessional;
}

bool SysInfoUtils::isSameUser(const QMimeData *data)
{
    const auto &userKey = QString(DFMGLOBAL_NAMESPACE::Mime::kDataUserIDKey) + "_" + QString::number(SysInfoUtils::getUserId());
    return data->hasFormat(userKey);
}

void SysInfoUtils::setMimeDataUserId(QMimeData *data)
{
    QByteArray userId;
    QString strUserID = QString::number(getUserId());
    userId.append(strUserID);
    data->setData(DFMGLOBAL_NAMESPACE::Mime::kDataUserIDKey, userId);

    QString strKey = QString(DFMGLOBAL_NAMESPACE::Mime::kDataUserIDKey) + "_" + strUserID;
    data->setData(strKey, userId);
}
