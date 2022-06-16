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
#include "discstatemanager.h"

#include "dfm-base/base/device/devicemanager.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/dbusservice/global_server_defines.h"

#include <QTimer>
#include <QDebug>

#include <mutex>

DPBURN_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

using namespace GlobalServerDefines;
static constexpr char kDiscPrefix[] { "/org/freedesktop/UDisks2/block_devices/sr" };

DiscStateManager *DiscStateManager::instance()
{
    static DiscStateManager manager;
    return &manager;
}

void DiscStateManager::initilaize()
{
    static std::once_flag flag;

    std::call_once(flag, [this]() {
        connect(DevProxyMng, &DeviceProxyManager::blockDevPropertyChanged,
                this, &DiscStateManager::onDevicePropertyChanged, Qt::DirectConnection);
        QTimer::singleShot(1000, this, &DiscStateManager::ghostMountForBlankDisc);
    });
}

void DiscStateManager::ghostMountForBlankDisc()
{
    auto &&devs = DevProxyMng->getAllBlockIds({});
    for (const auto &dev : devs) {
        if (dev.startsWith(kDiscPrefix))
            onDevicePropertyChanged(dev, DeviceProperty::kOptical, true);
    }
}

void DiscStateManager::onDevicePropertyChanged(const QString &id, const QString &propertyName, const QVariant &var)
{
    Q_UNUSED(var);

    // Query blank disc size
    if (id.startsWith(kDiscPrefix) && propertyName == DeviceProperty::kOptical && var.toBool()) {
        auto &&map = DevProxyMng->queryBlockInfo(id);
        bool blank { qvariant_cast<bool>(map[DeviceProperty::kOpticalBlank]) };
        qint64 curAvial { qvariant_cast<qint64>(map[DeviceProperty::kSizeFree]) };

        if (blank && curAvial == 0) {
            DevMngIns->mountBlockDevAsync(id, {}, [id](bool, DFMMOUNT::DeviceError, const QString &) {
                DevProxyMng->reloadOpticalInfo(id);
            });
        }
    }
}

DiscStateManager::DiscStateManager(QObject *parent)
    : QObject(parent)
{
}
