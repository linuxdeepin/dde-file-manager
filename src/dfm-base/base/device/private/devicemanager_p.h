// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEMANAGER_P_H
#define DEVICEMANAGER_P_H

#include <dfm-base/dfm_base_global.h>

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

    static bool isDaemonMountRunning();

    static void handleDlnfsMount(const QString &mpt, bool mount);
    static void unmountStackedMount(const QString &mpt);

    static DFMMOUNT::MountPassInfo askForPasswdWhenMountNetworkDevice(const QString &message, const QString &userDefault, const QString &domainDefault, const QString &uri);
    static int askForUserChoice(const QString &message, const QStringList &choices);

    static QString encryptPasswd(const QString &passwd);

private:
    DeviceWatcher *watcher { nullptr };
    DiscDeviceScanner *discScanner { nullptr };
    bool isWatching { false };
    bool autoMountBlock { false };
    QAtomicInteger<bool> isMountingOptical { false };

    DeviceManager *q { nullptr };
};

}

#endif   // DEVICEMANAGER_P_H
