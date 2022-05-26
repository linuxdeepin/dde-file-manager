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

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/base/device/deviceutils.h"

#include <QDebug>

DPCOMPUTER_USE_NAMESPACE
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

bool ComputerEventReceiver::handleShowPropertyDialog(const QList<QUrl> &urls)
{
    qDebug() << urls;
    bool handled = false;
    QList<QUrl> converted;
    for (const auto &url : urls) {
        if (url.scheme() == Global::kBurn && url.path().split("/", QString::SkipEmptyParts).size() == 3) {   // burn:///dev/sr0/disc_files/
            handled = true;
            QString id = url.path();
            id.remove("/dev/").remove("/disc_files").remove("/");
            id.prepend(kBlockDeviceIdPrefix);
            converted << ComputerUtils::makeBlockDevUrl(id);
        } else if (url.scheme() == Global::kFile) {
            QString devId;
            if (DevProxyMng->isMptOfDevice(url.path(), devId)) {
                handled = true;
                if (devId.startsWith(kBlockDeviceIdPrefix))
                    converted << ComputerUtils::makeBlockDevUrl(devId);
                else
                    converted << ComputerUtils::makeProtocolDevUrl(devId);
            }
        } else {
            converted << url;
        }
    }

    if (handled) {
        ComputerEventCaller::sendShowPropertyDialog(converted);
        qInfo() << "property is handled by computer plugin: " << urls << "after converted: " << converted;
    }
    return handled;
}

ComputerEventReceiver::ComputerEventReceiver(QObject *parent)
    : QObject(parent)
{
}
