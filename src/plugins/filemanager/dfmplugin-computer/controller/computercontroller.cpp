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
#include "computercontroller.h"
#include "events/computereventcaller.h"
#include "fileentity/appentryfileentity.h"
#include "utils/computerutils.h"

#include "dfm-base/utils/devicemanager.h"
#include "services/common/dialog/dialogservice.h"
#include <dfm-framework/framework.h>

#include <QDebug>
#include <QApplication>

DFMBASE_USE_NAMESPACE
DPCOMPUTER_USE_NAMESPACE

ComputerController *ComputerController::instance()
{
    static ComputerController ins(nullptr);
    return &ins;
}

void ComputerController::cdTo(quint64 winId, const QUrl &url)
{
    QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));

    // TODO(xust) get the info from factory
    DFMEntryFileInfoPointer info(new EntryFileInfo(url));
    if (!info) {
        qDebug() << "cannot create info of " << url;
        return;
    }

    auto target = info->targetUrl();
    if (target.isValid()) {
        ComputerEventCaller::cdTo(winId, target);
    } else {
        QString suffix = info->suffix();
        if (suffix == dfmbase::SuffixInfo::kBlock) {
            mountAndEnterBlockDevice(winId, info);
        } else if (suffix == dfmbase::SuffixInfo::kProtocol) {
        } else if (suffix == dfmbase::SuffixInfo::kStashedRemote) {
        } else if (suffix == dfmbase::SuffixInfo::kAppEntry) {
        }
    }
}

void ComputerController::requestMenu(quint64 winId, const QUrl &url, const QPoint &pos)
{
    qDebug() << "hello" << __FUNCTION__ << url << pos;
}

void ComputerController::rename(quint64 winId, const QUrl &url, const QString &name)
{
    qDebug() << "hello" << __FUNCTION__ << url << name;
}

void ComputerController::mountAndEnterBlockDevice(quint64 winId, const DFMEntryFileInfoPointer info)
{
    if (!info) {
        qDebug() << "a null info pointer is transfered";
        return;
    }

    bool isEncrypted = info->isEncrypted();
    bool isUnlocked = info->isUnlocked();
    QString shellId = ComputerUtils::getBlockDevIdByUrl(info->url());

    if (isEncrypted) {
        if (!isUnlocked) {
            auto &ctx = dpfInstance.serviceContext();
            auto dialogServ = ctx.service<DSC_NAMESPACE::DialogService>(DSC_NAMESPACE::DialogService::name());
            QString passwd = dialogServ->askPasswordForLockedDevice();
            if (passwd.isEmpty())
                return;

            DeviceManagerInstance.unlockAndDo(shellId, passwd, [dialogServ, winId](const QString &newID) {
                if (newID.isEmpty())
                    dialogServ->showErrorDialog(tr("Unlock device failed"), tr("Wrong password is inputed"));
                else
                    mountAndEnterBlockDevice(winId, newID);
            });
        } else {
            auto realDevId = info->clearDeviceId();
            mountAndEnterBlockDevice(winId, realDevId);
        }
    } else {
        mountAndEnterBlockDevice(winId, shellId);
    }
}

void ComputerController::mountAndEnterBlockDevice(quint64 winId, const QString &id)
{
    QFuture<QString> fu = QtConcurrent::run(&DeviceManagerInstance, &DeviceManager::invokeMountBlockDevice, id);
    QFutureWatcher<QString> *watcher = new QFutureWatcher<QString>();
    connect(watcher, &QFutureWatcher<QString>::finished, [watcher, id, winId] {
        QString path = watcher->result();
        qDebug() << "cd to: " + path << ", " << id;
        ComputerEventCaller::cdTo(winId, path);
        watcher->deleteLater();
    });
    watcher->setFuture(fu);
}

ComputerController::ComputerController(QObject *parent)
    : QObject(parent)
{
}
