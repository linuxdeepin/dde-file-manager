/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: zhangsheng<zhangsheng@uniontech.com>
 *             lvwujun<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
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
#include "bluetoothservice.h"
#include "private/bluetoothmanager.h"

#include "dfm-base/utils/dialogmanager.h"

DSC_BEGIN_NAMESPACE

bool BluetoothService::bluetoothEnable()
{
    return Q_LIKELY(BluetoothManager::instance()->bluetoothSendEnable()) && BluetoothManager::instance()->hasAdapter();
}

void BluetoothService::sendFiles(const QStringList &paths, BluetoothTransDialog::TransferMode mode, const QString &deviceId)
{
    if (!BluetoothTransDialog::isBluetoothIdle()) {
        DialogManagerInstance->showMessageDialog(dfmbase::DialogManager::kMsgInfo, tr("Sending files now, please try later."));
        return;
    }

    if (paths.isEmpty()) {
        qDebug() << "bluetooth: cannot send empty files";
        return;
    }

    BluetoothTransDialog *dlg = new BluetoothTransDialog(paths, mode, deviceId);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

BluetoothService::BluetoothService(QObject *parent)
    : dpf::PluginService(parent), dpf::AutoServiceRegister<BluetoothService>()
{
}

BluetoothService::~BluetoothService()
{
}

DSC_END_NAMESPACE
