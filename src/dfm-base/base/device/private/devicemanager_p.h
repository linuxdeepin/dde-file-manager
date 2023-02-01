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
#ifndef DEVICEMANAGER_P_H
#define DEVICEMANAGER_P_H

#include "dfm-base/dfm_base_global.h"

#include <QTimer>
#include <QHash>

#include <dfm-mount/base/dmount_global.h>
#include <dfm-mount/dprotocoldevice.h>

namespace dfmbase {

class DeviceWatcher;
class DeviceManager;
class DiscDeviceScanner;
class DeviceManagerPrivate
{
    friend DeviceManager;
    Q_DISABLE_COPY(DeviceManagerPrivate)

public:
    explicit DeviceManagerPrivate(DeviceManager *qq);

private:
    // private operations
    void mountAllBlockDev();
    void mountDlnfsOnHome();

    static DFMMOUNT::MountPassInfo askForPasswdWhenMountNetworkDevice(const QString &message, const QString &userDefault, const QString &domainDefault, const QString &uri);
    static int askForUserChoice(const QString &message, const QStringList &choices);

private:
    DeviceWatcher *watcher { nullptr };
    DiscDeviceScanner *discScanner { nullptr };
    bool isWatching { false };
    QAtomicInteger<bool> isMountingOptical { false };

    DeviceManager *q { nullptr };
};

}

#endif   // DEVICEMANAGER_P_H
