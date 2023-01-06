// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
