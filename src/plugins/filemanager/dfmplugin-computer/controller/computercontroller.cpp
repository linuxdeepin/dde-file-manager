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
#include "dfm-base/file/entry/entryfileinfo.h"
#include "dfm-base/dfm_event_defines.h"
#include "services/common/dialog/dialogservice.h"
#include <dfm-framework/framework.h>

#include <QDebug>
#include <QApplication>
#include <QMenu>

DFMBASE_USE_NAMESPACE
DPCOMPUTER_USE_NAMESPACE

ComputerController *ComputerController::instance()
{
    static ComputerController instance;
    return &instance;
}

void ComputerController::onOpenItem(quint64 winId, const QUrl &url)
{
    // TODO(xust) get the info from factory
    DFMEntryFileInfoPointer info(new EntryFileInfo(url));
    if (!info) {
        qDebug() << "cannot create info of " << url;
        QApplication::setOverrideCursor(Qt::ArrowCursor);
        return;
    }

    if (!info->isAccessable()) {
        qDebug() << "cannot access device: " << url;
        QApplication::setOverrideCursor(Qt::ArrowCursor);
        return;
    }

    auto target = info->targetUrl();
    if (target.isValid()) {
        ComputerEventCaller::cdTo(winId, target);
    } else {
        QString suffix = info->suffix();
        if (suffix == dfmbase::SuffixInfo::kBlock) {
            mountDevice(winId, info);
        } else if (suffix == dfmbase::SuffixInfo::kProtocol) {
        } else if (suffix == dfmbase::SuffixInfo::kStashedRemote) {
        } else if (suffix == dfmbase::SuffixInfo::kAppEntry) {
        }
    }
}

void ComputerController::onMenuRequest(quint64 winId, const QUrl &url, bool triggerFromSidebar)
{
    DFMEntryFileInfoPointer info(new EntryFileInfo(url));
    QMenu *menu = info->createMenu();
    if (menu) {
        connect(menu, &QMenu::triggered, [=](QAction *act) {
            QString actText = act->text();
            actionTriggered(url, winId, actText, triggerFromSidebar);
        });
        menu->exec(QCursor::pos());
        menu->deleteLater();
    }
}

void ComputerController::doRename(quint64 winId, const QUrl &url, const QString &name)
{
    Q_UNUSED(winId);

    DFMEntryFileInfoPointer info(new EntryFileInfo(url));
    if (info->removable() && info->suffix() == SuffixInfo::kBlock) {
        QString devId = ComputerUtils::getBlockDevIdByUrl(url);   // for now only block devices can be renamed.
        DeviceManagerInstance.invokeRenameBlockDevice(devId, name);
        return;
    }

    if (!info->removable())
        doSetAlias(info, name);
}

void ComputerController::doSetAlias(DFMEntryFileInfoPointer info, const QString &alias)
{
    // TODO(xust)}
}

void ComputerController::mountDevice(quint64 winId, const DFMEntryFileInfoPointer info, ActionAfterMount act)
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
            QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
            auto &ctx = dpfInstance.serviceContext();
            auto dialogServ = ctx.service<DSC_NAMESPACE::DialogService>(DSC_NAMESPACE::DialogService::name());
            QString passwd = dialogServ->askPasswordForLockedDevice();
            if (passwd.isEmpty()) {
                QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
                return;
            }
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

            DeviceManagerInstance.unlockAndDo(shellId, passwd, [=](const QString &newID) {
                if (newID.isEmpty())
                    dialogServ->showErrorDialog(tr("Unlock device failed"), tr("Wrong password is inputed"));
                else
                    mountDevice(winId, newID, act);
            });
        } else {
            auto realDevId = info->clearDeviceId();
            mountDevice(winId, realDevId, act);
        }
    } else {
        mountDevice(winId, shellId, act);
    }
}

void ComputerController::mountDevice(quint64 winId, const QString &id, ActionAfterMount act)
{
    QFutureWatcher<QString> *watcher = new QFutureWatcher<QString>();
    connect(watcher, &QFutureWatcher<QString>::finished, [=] {
        QString path = watcher->result();
        QUrl u;
        u.setScheme(SchemeTypes::kFile);
        u.setPath(path);
        if (act == kEnterDirectory)
            ComputerEventCaller::cdTo(winId, path);
        else if (act == kEnterInNewWindow)
            ComputerEventCaller::sendEnterInNewWindow(u);
        else if (act == kEnterInNewTab)
            ComputerEventCaller::sendEnterInNewTab(winId, u);
        watcher->deleteLater();
    });
    QFuture<QString> fu = QtConcurrent::run(&DeviceManagerInstance, &DeviceManager::invokeMountBlockDevice, id);
    watcher->setFuture(fu);
    QApplication::setOverrideCursor(Qt::ArrowCursor);
}

void ComputerController::actionTriggered(const QUrl &url, quint64 winId, const QString &actionText, bool triggerFromSidebar)
{
    DFMEntryFileInfoPointer info(new EntryFileInfo(url));

    // if not original supported suffix, publish event to notify subscribers to handle
    QString sfx = info->suffix();
    if (sfx != SuffixInfo::kBlock
        && sfx != SuffixInfo::kProtocol
        && sfx != SuffixInfo::kUserDir
        && sfx != SuffixInfo::kAppEntry
        && sfx != SuffixInfo::kStashedRemote) {
        ComputerEventCaller::sendContextActionTriggered(url, actionText);
        return;
    }

    if (actionText == ContextMenuActionTrs::trOpenInNewWin())
        actOpenInNewWindow(winId, info);
    else if (actionText == ContextMenuActionTrs::trOpenInNewTab())
        actOpenInNewTab(winId, info);
    else if (actionText == ContextMenuActionTrs::trMount())
        actMount(info);
    else if (actionText == ContextMenuActionTrs::trUnmount())
        actUnmount(info);
    else if (actionText == ContextMenuActionTrs::trRename())
        actRename(winId, info, triggerFromSidebar);
    else if (actionText == ContextMenuActionTrs::trFormat())
        actFormat(winId, info);
    else if (actionText == ContextMenuActionTrs::trSafelyRemove())
        actSafelyRemove(info);
    else if (actionText == ContextMenuActionTrs::trEject())
        actEject(url);
    else if (actionText == ContextMenuActionTrs::trProperties())
        actProperties(url);
}

void ComputerController::actEject(const QUrl &url)
{
    QString id;
    if (url.path().endsWith(SuffixInfo::kBlock)) {
        id = ComputerUtils::getBlockDevIdByUrl(url);
        DeviceManagerInstance.invokeDetachBlockDevice(id);
    } else if (url.path().endsWith(SuffixInfo::kProtocol)) {
        id = ComputerUtils::getProtocolDevIdByUrl(url);
        DeviceManagerInstance.invokeDetachProtocolDevice(id);
    } else {
        qDebug() << url << "is not support " << __FUNCTION__;
    }
}

void ComputerController::actOpenInNewWindow(quint64 winId, DFMEntryFileInfoPointer info)
{
    auto target = info->targetUrl();
    if (target.isValid())
        ComputerEventCaller::sendEnterInNewWindow(target);
    else
        mountDevice(winId, info, kEnterInNewWindow);
}

void ComputerController::actOpenInNewTab(quint64 winId, DFMEntryFileInfoPointer info)
{
    auto target = info->targetUrl();
    if (target.isValid())
        ComputerEventCaller::sendEnterInNewTab(winId, target);
    else
        mountDevice(winId, info, kEnterInNewTab);
}

void ComputerController::actMount(DFMEntryFileInfoPointer info)
{
    mountDevice(0, info, kNone);
}

void ComputerController::actUnmount(DFMEntryFileInfoPointer info)
{
    QString devId;
    if (info->suffix() == SuffixInfo::kBlock) {
        devId = ComputerUtils::getBlockDevIdByUrl(info->url());
        DeviceManagerInstance.invokeUnmountBlockDevice(devId);
    } else if (info->suffix() == SuffixInfo::kProtocol) {
        devId = ComputerUtils::getProtocolDevIdByUrl(info->url());
        DeviceManagerInstance.invokeUnmountProtocolDevice(devId);
    } else {
        qDebug() << info->url() << "is not support " << __FUNCTION__;
    }
}

void ComputerController::actSafelyRemove(DFMEntryFileInfoPointer info)
{
    actEject(info->url());
}

void ComputerController::actRename(quint64 winId, DFMEntryFileInfoPointer info, bool triggerFromSidebar)
{
    if (!info) {
        qWarning() << "info is not valid!" << __FUNCTION__;
        return;
    }

    if (info->removable() && info->targetUrl().isValid()) {
        qWarning() << "cannot rename a mounted device! " << __FUNCTION__;
        return;
    }

    if (!triggerFromSidebar)
        Q_EMIT requestRename(winId, info->url());
    else
        ComputerUtils::sbIns()->triggerItemEdit(winId, info->url());
}

void ComputerController::actFormat(quint64 winId, DFMEntryFileInfoPointer info)
{
    if (info->suffix() != SuffixInfo::kBlock) {
        qWarning() << "non block device is not support format" << info->url();
        return;
    }
    auto url = info->url();
    QString devDesc = "/dev/" + url.path().remove("." + QString(SuffixInfo::kBlock));
    qDebug() << devDesc;

    QString cmd = "dde-device-formatter";
    QStringList args;
    args << "-m=" + QString::number(winId) << devDesc;

    QProcess::startDetached(cmd, args);
}

void ComputerController::actRemove(DFMEntryFileInfoPointer info)
{
    // TODO(xust)
}

void ComputerController::actProperties(const QUrl &url)
{
    // TODO(xust)
}

ComputerController::ComputerController(QObject *parent)
    : QObject(parent)
{
}
