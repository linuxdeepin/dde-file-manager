// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRTUALBLUETOOTHPLUGIN_H
#define VIRTUALBLUETOOTHPLUGIN_H

#include "dfmplugin_utils_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_utils {

class VirtualBluetoothPlugin : public dpf::Plugin
{
    Q_OBJECT
    DPF_EVENT_NAMESPACE(DPUTILS_NAMESPACE)

    DPF_EVENT_REG_SLOT(slot_Bluetooth_IsAvailable)
    DPF_EVENT_REG_SLOT(slot_Bluetooth_SendFiles)

public:
    virtual void initialize() override;
    virtual bool start() override;

private Q_SLOTS:
    bool bluetoothAvailable();
    void sendFiles(const QStringList &paths, const QString &deviceId = "");
};

}   // namespace dfmplugin_utils

#endif   // VIRTUALBLUETOOTHPLUGIN_H
