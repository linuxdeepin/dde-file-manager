// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "discstatemanager.h"

#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <QTimer>
#include <QDebug>

#include <mutex>

using namespace dfmplugin_burn;
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
            DevMngIns->mountBlockDevAsync(id, {}, [id](bool, const DFMMOUNT::OperationErrorInfo &, const QString &) {
                DevProxyMng->reloadOpticalInfo(id);
            });
        }
    }
}

DiscStateManager::DiscStateManager(QObject *parent)
    : QObject(parent)
{
}
