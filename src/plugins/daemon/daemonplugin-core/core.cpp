/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "core.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>

DAEMONPCORE_USE_NAMESPACE

static constexpr char kDaemonServicePath[] { "com.deepin.filemanager.daemon" };

bool Core::start()
{
    QDBusConnection connection = QDBusConnection::systemBus();
    if (!connection.interface()->isServiceRegistered(kDaemonServicePath)) {
        qInfo() << connection.registerService(kDaemonServicePath) << "register" << kDaemonServicePath << "success";
        // TODO(zhangs): add detail dbus
    } else {
        qWarning() << connection.registerService(kDaemonServicePath) << "register" << kDaemonServicePath << "failed";
    }

    return true;
}
