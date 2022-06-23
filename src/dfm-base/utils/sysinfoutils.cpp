/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#include "sysinfoutils.h"
#include "dfm_global_defines.h"

#include <DSysInfo>

#include <QDBusInterface>
#include <QDBusReply>

#include <unistd.h>

DCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

QString SysInfoUtils::getUser()
{
    static QString user = QString::fromLocal8Bit(qgetenv("USER"));

    return user;
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
    if (data->hasFormat(DFMGLOBAL_NAMESPACE::Mime::kMimeDataUserIDKey))
        return data->data(DFMGLOBAL_NAMESPACE::Mime::kMimeDataUserIDKey) == QString::number(SysInfoUtils::getUserId());

    return false;
}
