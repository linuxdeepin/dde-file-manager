// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virtualbluetoothplugin.h"
#include "private/bluetoothmanager.h"
#include "private/bluetoothtransdialog.h"

#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/sysinfoutils.h>

#include <QTimer>

using namespace dfmplugin_utils;

void VirtualBluetoothPlugin::initialize()
{
    if (DFMBASE_NAMESPACE::SysInfoUtils::isOpenAsAdmin()) {
        fmWarning() << "Bluetooth is disable when open as admin";
        return;
    }

    QTimer::singleShot(1000, this, [] {
        fmInfo() << "start initialize bluetooth manager";
        BluetoothManager::instance();
        fmInfo() << "end initialize bluetooth manager";
    });

    dpfSlotChannel->connect("dfmplugin_utils", "slot_Bluetooth_IsAvailable", this, &VirtualBluetoothPlugin::bluetoothAvailable);
    dpfSlotChannel->connect("dfmplugin_utils", "slot_Bluetooth_SendFiles", this, &VirtualBluetoothPlugin::sendFiles);
}

bool VirtualBluetoothPlugin::start()
{
    return true;
}

bool VirtualBluetoothPlugin::bluetoothAvailable()
{
    return BluetoothManager::instance()->bluetoothSendEnable() && BluetoothManager::instance()->hasAdapter();
}

void VirtualBluetoothPlugin::sendFiles(const QStringList &paths, const QString &deviceId)
{
    if (!BluetoothTransDialog::isBluetoothIdle()) {
        DialogManagerInstance->showMessageDialog(tr("Sending files now, please try later."));
        return;
    }

    if (paths.isEmpty()) {
        fmDebug() << "bluetooth: cannot send empty files";
        return;
    }

    BluetoothTransDialog *dlg = new BluetoothTransDialog(paths, deviceId);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}
