// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virtualbluetoothplugin.h"
#include "private/bluetoothmanager.h"
#include "private/bluetoothtransdialog.h"

#include <dfm-base/utils/dialogmanager.h>

using namespace dfmplugin_utils;

void VirtualBluetoothPlugin::initialize()
{
    //    BluetoothManager::instance();

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
        DialogManagerInstance->showMessageDialog(DFMBASE_NAMESPACE::DialogManager::kMsgInfo, tr("Sending files now, please try later."));
        return;
    }

    if (paths.isEmpty()) {
        qDebug() << "bluetooth: cannot send empty files";
        return;
    }

    BluetoothTransDialog *dlg = new BluetoothTransDialog(paths, deviceId);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}
