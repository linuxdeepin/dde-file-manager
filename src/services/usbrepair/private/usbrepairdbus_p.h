// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USBREPAIRDBUS_P_H
#define USBREPAIRDBUS_P_H

#include "service_usbrepair_global.h"
#include "usbrepairadaptor.h"
#include "usbrepairmonitor.h"
#include "usbrepairworker.h"

class UsbRepairDBus;

SERVICEUSBREPAIR_BEGIN_NAMESPACE

class UsbRepairDBusPrivate
{
    friend class ::UsbRepairDBus;

public:
    explicit UsbRepairDBusPrivate(UsbRepairDBus *qq)
        : q(qq),
          adapter(new UsbRepairAdaptor(qq)),
          monitor(new UsbRepairMonitor(qq)),
          worker(new UsbRepairWorker(qq))
    {
        initConnect();
    }
    ~UsbRepairDBusPrivate() { }

    void initConnect();

private:
    UsbRepairDBus *q { nullptr };
    UsbRepairAdaptor *adapter { nullptr };
    UsbRepairMonitor *monitor { nullptr };
    UsbRepairWorker *worker { nullptr };
};

SERVICEUSBREPAIR_END_NAMESPACE

#endif   // USBREPAIRDBUS_P_H
