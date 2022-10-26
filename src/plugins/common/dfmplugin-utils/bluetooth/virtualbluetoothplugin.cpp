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
#include "virtualbluetoothplugin.h"
#include "private/bluetoothmanager.h"
#include "private/bluetoothtransdialog.h"

#include "dfm-base/utils/dialogmanager.h"

using namespace dfmplugin_utils;

void VirtualBluetoothPlugin::initialize()
{
    BluetoothManager::instance();

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
