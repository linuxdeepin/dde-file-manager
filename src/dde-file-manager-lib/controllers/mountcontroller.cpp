/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

#include "mountcontroller.h"

#include "app/define.h"
#include "views/windowmanager.h"
#include "models/mountfileinfo.h"

#include "dfmeventdispatcher.h"
#include "ddiskmanager.h"
#include "dblockdevice.h"

#include <QTimer>
#include <QWidget>

DFM_USE_NAMESPACE

MountController::MountController(QObject *parent) : DAbstractFileController(parent)
{

}

MountController::~MountController()
{

}

const DAbstractFileInfoPointer MountController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
{
    MountFileInfo *info = new MountFileInfo(event->url());

    return DAbstractFileInfoPointer(info);
}

const QList<DAbstractFileInfoPointer> MountController::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
    QString deviceUrlStr = event->url().fragment();
    if (deviceUrlStr.isEmpty()) {
        return {};
    }

    DUrl deviceUrl(deviceUrlStr);
    DUrl mountPointUrl; // just a init value

    if (deviceUrl.scheme() == DEVICE_SCHEME) {
        //
    } else if (deviceUrl.scheme() == "udisks") {
        // for test:  mount://test#udisks:///org/freedesktop/UDisks2/block_devices/sda1
        QScopedPointer<DBlockDevice> blDev(DDiskManager::createBlockDevice(deviceUrl.path()));
        if (!blDev || !blDev->hasFileSystem() || blDev->isEncrypted()) {
            return {};
        }
        QString mountPoint;

        if (blDev->mountPoints().isEmpty()) {
            mountPoint = blDev->mount({});
        } else {
            mountPoint = blDev->mountPoints().first();
        }

        if (!mountPoint.isEmpty()) {
            mountPointUrl = DUrl(QUrl::fromLocalFile(mountPoint));
        }
    }

    // Do change directory
    if (mountPointUrl.isValid()) {
        QWidget *p = WindowManager::getWindowById(event->windowId());
        QTimer::singleShot(0, p, [p, mountPointUrl] {
            DFMEventDispatcher::instance()->processEvent<DFMChangeCurrentUrlEvent>(nullptr, mountPointUrl, p);
        });
    }

    return {};
}
