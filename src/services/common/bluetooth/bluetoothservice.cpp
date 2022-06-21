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
#include "bluetooth_defines.h"
#include "private/bluetoothmanager.h"
#include "private/bluetoothtransdialog.h"

#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/utils/universalutils.h"

#include <dfm-framework/framework.h>

#include <QUrl>

#include <mutex>

DSC_BEGIN_NAMESPACE

namespace EventType {
const int kSendFiles = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
}   // namespace EventType

BluetoothService *BluetoothService::service()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(name()))
            abort();
    });

    return ctx.service<BluetoothService>(name());
}

bool BluetoothService::bluetoothEnable()
{
    return Q_LIKELY(BluetoothManager::instance()->bluetoothSendEnable()) && BluetoothManager::instance()->hasAdapter();
}

void BluetoothService::sendFiles(const QList<QUrl> &urls)
{
    QStringList paths;
    for (const QUrl &url : urls)
        paths << url.path();
    sendFiles(paths);
}

void BluetoothService::sendFiles(const QStringList &paths, TransferMode mode, const QString &deviceId)
{
    if (!BluetoothTransDialog::isBluetoothIdle()) {
        DialogManagerInstance->showMessageDialog(DFMBASE_NAMESPACE::DialogManager::kMsgInfo, tr("Sending files now, please try later."));
        return;
    }

    if (paths.isEmpty()) {
        qDebug() << "bluetooth: cannot send empty files";
        return;
    }

    BluetoothTransDialog *dlg = new BluetoothTransDialog(paths, static_cast<BluetoothTransDialog::TransferMode>(mode), deviceId);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

BluetoothService::BluetoothService(QObject *parent)
    : dpf::PluginService(parent), dpf::AutoServiceRegister<BluetoothService>()
{
    dpfSignalDispatcher->subscribe(EventType::kSendFiles, this, static_cast<void (BluetoothService::*)(const QList<QUrl> &)>(&BluetoothService::sendFiles));
}

BluetoothService::~BluetoothService()
{
}

DSC_END_NAMESPACE
