/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "computereventreceiver.h"
#include "computereventcaller.h"
#include "controller/computercontroller.h"
#include "utils/computerutils.h"

#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/universalutils.h"

#include <QDebug>

using namespace dfmplugin_computer;
DFMBASE_USE_NAMESPACE

ComputerEventReceiver *ComputerEventReceiver::instance()
{
    static ComputerEventReceiver ins;
    return &ins;
}

void ComputerEventReceiver::handleItemEject(const QUrl &url)
{
    ComputerControllerInstance->actEject(url);
}

bool ComputerEventReceiver::handleSepateTitlebarCrumb(const QUrl &url, QList<QVariantMap> *mapGroup)
{
    Q_ASSERT(mapGroup);
    if (url.scheme() == ComputerUtils::scheme()) {
        QVariantMap map;
        map["CrumbData_Key_Url"] = url;
        map["CrumbData_Key_DisplayText"] = tr("Computer");
        map["CrumbData_Key_IconName"] = ComputerUtils::icon().name();
        mapGroup->push_back(map);
        return true;
    }

    return false;
}

bool ComputerEventReceiver::handleSortItem(const QString &group, const QString &subGroup, const QUrl &a, const QUrl &b)
{
    if (!(group == "Group_Device" || group == "Group_Network"))
        return false;

    if (!(subGroup == Global::Scheme::kComputer || subGroup == Global::Scheme::kSmb || subGroup == Global::Scheme::kFtp))
        return false;

    return ComputerUtils::sortItem(a, b);
}

bool ComputerEventReceiver::handleSetTabName(const QUrl &url, QString *tabName)
{
    // if the url is the mount point of inner disk, set it to alias if alias if not empty.
    auto devs = DevProxyMng->getAllBlockIds(GlobalServerDefines::kMounted | GlobalServerDefines::kSystem);
    for (const auto &dev : devs) {
        auto devInfo = DevProxyMng->queryBlockInfo(dev);
        auto mpt = QUrl::fromLocalFile(devInfo.value(GlobalServerDefines::DeviceProperty::kMountPoint).toString());
        if (UniversalUtils::urlEquals(mpt, url)) {
            auto info = InfoFactory::create<EntryFileInfo>(ComputerUtils::makeBlockDevUrl(dev));
            if (info) {
                *tabName = info->displayName();
                return true;
            }
        }
    }
    return false;
}

void ComputerEventReceiver::setContextMenuEnable(bool enable)
{
    ComputerUtils::contextMenuEnabled = enable;
}

ComputerEventReceiver::ComputerEventReceiver(QObject *parent)
    : QObject(parent)
{
}
