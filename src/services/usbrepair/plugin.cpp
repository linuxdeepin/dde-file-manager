// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbrepairdbus.h"

static UsbRepairDBus *usbRepairDBus = nullptr;

// DEBUG:
// 1. build a debug so file and copy to install path
// 2. sudo systemctl stop deepin-service-plugin@org.deepin.Filemanager.UsbRepair.service
// 3. launch app: sudo /usr/bin/deepin-service-manager -g app

extern "C" int DSMRegister(const char *name, void *data)
{
    (void)data;
    usbRepairDBus = new UsbRepairDBus(name);
    return 0;
}

extern "C" int DSMUnRegister(const char *name, void *data)
{
    (void)name;
    (void)data;
    if (usbRepairDBus) {
        usbRepairDBus->cleanup();
        usbRepairDBus->deleteLater();
        usbRepairDBus = nullptr;
    }
    return 0;
}
