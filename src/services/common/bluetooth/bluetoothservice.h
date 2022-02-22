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
#ifndef BLUETOOTHSERVICE_H
#define BLUETOOTHSERVICE_H

#include "dfm_common_service_global.h"
#include "private/bluetoothtransdialog.h"

#include <dfm-framework/service/pluginservicecontext.h>

DSC_BEGIN_NAMESPACE

class BluetoothService final : public dpf::PluginService, dpf::AutoServiceRegister<BluetoothService>
{
    Q_OBJECT
    Q_DISABLE_COPY(BluetoothService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    static QString name()
    {
        return "org.deepin.service.BluetoothService";
    }

    bool bluetoothEnable();
    void sendFiles(const QStringList &paths, BluetoothTransDialog::TransferMode mode = BluetoothTransDialog::kSelectDeviceToSend, const QString &deviceId = {});

private:
    explicit BluetoothService(QObject *parent = nullptr);
    virtual ~BluetoothService() override;
};

DSC_END_NAMESPACE

#endif   // BLUETOOTHSERVICE_H
