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

void ComputerEventReceiver::setContextMenuEnable(bool enable)
{
    ComputerUtils::contextMenuEnabled = enable;
}

ComputerEventReceiver::ComputerEventReceiver(QObject *parent)
    : QObject(parent)
{
}
