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
#include "fileentity/stashedprotocolentryfileentity.h"
#include "fileentity/blockentryfileentity.h"
#include "utils/computerutils.h"
#include "utils/stashmountsutils.h"
#include "utils/remotepasswdmanager.h"
#include "watcher/computeritemwatcher.h"

#include "services/common/propertydialog/property_defines.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/base/device/devicemanager.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/file/entry/entryfileinfo.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/dbusservice/global_server_defines.h"

#include <dfm-framework/framework.h>

#include <QDebug>
#include <QApplication>
#include <QMenu>
#include <QProcess>
#include <QThread>

DFMBASE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DPCOMPUTER_USE_NAMESPACE
using namespace GlobalServerDefines;

ComputerController *ComputerController::instance()
{
    static ComputerController instance;
    return &instance;
}

void ComputerController::onOpenItem(quint64 winId, const QUrl &url)
{
    DFMEntryFileInfoPointer info(new EntryFileInfo(url));
    if (!info) {
        qDebug() << "cannot create info of " << url;
        ComputerUtils::setCursorState();
        return;
    }

    DFMBASE_USE_NAMESPACE;
    QString suffix = info->suffix();
    if (!ComputerUtils::isPresetSuffix(suffix)) {
        ComputerEventCaller::sendOpenItem(winId, info->url());
        return;
    }

    bool isOptical = info->extraProperty(DeviceProperty::kOptical).toBool();
    if (!info->isAccessable() && !isOptical) {
        qDebug() << "cannot access device: " << url;
        bool needAskForFormat = info->suffix() == SuffixInfo::kBlock
                && !info->extraProperty(DeviceProperty::kHasFileSystem).toBool()
                && !info->extraProperty(DeviceProperty::kIsEncrypted).toBool()
                && !info->extraProperty(DeviceProperty::kOpticalDrive).toBool();
        if (needAskForFormat) {
            if (DialogManagerInstance->askForFormat())
                actFormat(winId, info);
        }
        ComputerUtils::setCursorState();
        return;
    }

    auto target = info->targetUrl();
    if (target.isValid()) {
        if (isOptical)
            target = ComputerUtils::makeBurnUrl(ComputerUtils::getBlockDevIdByUrl(url));
        ComputerEventCaller::cdTo(winId, target);
    } else {
        if (suffix == SuffixInfo::kBlock) {
            mountDevice(winId, info);
        } else if (suffix == SuffixInfo::kProtocol) {
            ;
        } else if (suffix == SuffixInfo::kStashedProtocol) {
            actMount(winId, info, true);
        } else if (suffix == SuffixInfo::kAppEntry) {
            QString cmd = info->extraProperty(ExtraPropertyName::kExecuteCommand).toString();
            QProcess::startDetached(cmd);
        }
    }
}

void ComputerController::onMenuRequest(quint64 winId, const QUrl &url, bool triggerFromSidebar)
{
    if (!ComputerUtils::contextMenuEnabled)
        return;

    DFMEntryFileInfoPointer info(new EntryFileInfo(url));
    QMenu *menu = info->createMenu();
    if (menu) {
        connect(menu, &QMenu::triggered, [=](QAction *act) {
            QString actText = act->text();
            actionTriggered(info, winId, actText, triggerFromSidebar);
        });
        menu->exec(QCursor::pos());
        menu->deleteLater();
    }
}

void ComputerController::doRename(quint64 winId, const QUrl &url, const QString &name)
{
    Q_UNUSED(winId);

    DFMEntryFileInfoPointer info(new EntryFileInfo(url));
    bool removable = info->extraProperty(DeviceProperty::kRemovable).toBool();
    if (removable && info->suffix() == SuffixInfo::kBlock) {
        if (info->displayName() == name)
            return;
        ComputerUtils::setCursorState(true);
        QString devId = ComputerUtils::getBlockDevIdByUrl(url);   // for now only block devices can be renamed.
        DevMngIns->renameBlockDevAsync(devId, name, {}, [=](bool ok, DFMMOUNT::DeviceError err) {
            ComputerUtils::setCursorState();
            if (!ok) {
                qInfo() << "rename block device failed: " << devId << err;
            }
        });
        return;
    }

    if (!removable) {
        doSetAlias(info, name);
    }
}

void ComputerController::doSetAlias(DFMEntryFileInfoPointer info, const QString &alias)
{
    QString uuid = info->extraProperty(DeviceProperty::kUUID).toString();
    if (uuid.isEmpty()) {
        qWarning() << "params exception!" << info->url();
        return;
    }

    using namespace BlockAdditionalProperty;
    QString displayAlias = alias.trimmed();
    QString displayName = info->displayName();
    QVariantList list = Application::genericSetting()->value(kAliasGroupName, kAliasItemName).toList();

    // [a] empty alias  -> remove from list
    // [b] exists alias -> cover it
    // [c] not exists   -> append
    bool exists = false;
    for (int i = 0; i < list.count(); i++) {
        QVariantMap map = list.at(i).toMap();
        if (map.value(kAliasItemUUID).toString() == uuid) {
            if (displayAlias.isEmpty()) {   // [a]
                list.removeAt(i);
            } else {   // [b]
                map[kAliasItemName] = displayName;
                map[kAliasItemAlias] = displayAlias;
                list[i] = map;
            }
            exists = true;
            break;
        }
    }

    // [c]
    if (!exists && !displayAlias.isEmpty() && !uuid.isEmpty()) {
        QVariantMap map;
        map[kAliasItemUUID] = uuid;
        map[kAliasItemName] = displayName;
        map[kAliasItemAlias] = displayAlias;
        list.append(map);
        qInfo() << "append setting item: " << map;
    }

    Application::genericSetting()->setValue(kAliasGroupName, kAliasItemName, list);

    // update sidebar and computer display
    QString sidebarName = displayAlias.isEmpty() ? info->displayName() : displayAlias;
    SideBarService::service()->updateItemName(info->url(), sidebarName, true);
    Q_EMIT updateItemAlias(info->url());
}

void ComputerController::mountDevice(quint64 winId, const DFMEntryFileInfoPointer info, ActionAfterMount act)
{
    if (!info) {
        qCritical() << "a null info pointer is transfered";
        return;
    }

    bool isEncrypted = info->extraProperty(DeviceProperty::kIsEncrypted).toBool();
    bool isUnlocked = info->extraProperty(DeviceProperty::kCleartextDevice).toString().length() > 1;
    QString shellId = ComputerUtils::getBlockDevIdByUrl(info->url());
    bool hasFileSystem = info->extraProperty(DeviceProperty::kHasFileSystem).toBool();
    bool isOpticalDrive = info->extraProperty(DeviceProperty::kOpticalDrive).toBool();

    bool needAskForFormat = info->suffix() == SuffixInfo::kBlock
            && !hasFileSystem
            && !isEncrypted
            && !isOpticalDrive;
    if (needAskForFormat) {
        if (DialogManagerInstance->askForFormat())
            actFormat(winId, info);
        return;
    }

    bool isOptical = info->extraProperty(DeviceProperty::kOptical).toBool();
    if (isOpticalDrive && !isOptical)
        return;

    if (isEncrypted) {
        if (!isUnlocked) {
            ComputerUtils::setCursorState();
            QString passwd = DialogManagerInstance->askPasswordForLockedDevice();
            if (passwd.isEmpty()) {
                ComputerUtils::setCursorState();
                return;
            }
            ComputerUtils::setCursorState(true);

            DevMngIns->unlockBlockDevAsync(shellId, passwd, {}, [=](bool ok, DFMMOUNT::DeviceError err, const QString &newId) {
                ComputerUtils::setCursorState();

                if (ok) {
                    this->mountDevice(winId, newId, shellId, act);
                } else {
                    DialogManagerInstance->showErrorDialog(tr("Unlock device failed"), tr("Wrong password is inputed"));
                    qInfo() << "unlock device failed: " << shellId << err;
                }
            });
        } else {
            auto realDevId = info->extraProperty(DeviceProperty::kCleartextDevice).toString();
            mountDevice(winId, realDevId, shellId, act);
        }
    } else {
        auto realId = shellId;
        mountDevice(winId, realId, "", act);
    }
}

void ComputerController::mountDevice(quint64 winId, const QString &id, const QString &shellId, ActionAfterMount act)
{
    auto cdTo = [](const QString &id, const QUrl &u, quint64 winId, ActionAfterMount act) {
        ComputerItemWatcherInstance->insertUrlMapper(id, u);

        if (act == kEnterDirectory)
            ComputerEventCaller::cdTo(winId, u);
        else if (act == kEnterInNewWindow)
            ComputerEventCaller::sendEnterInNewWindow(u);
        else if (act == kEnterInNewTab)
            ComputerEventCaller::sendEnterInNewTab(winId, u);
    };

    const auto &&data = DevProxyMng->queryBlockInfo(id);
    if (data.value(DeviceProperty::kOpticalDrive).toBool() && data.value(DeviceProperty::kOpticalBlank).toBool()) {
        if (!data.value(DeviceProperty::kOpticalWriteSpeed).toStringList().isEmpty()) {   // already load data from xorriso.
            cdTo(id, ComputerUtils::makeBurnUrl(id), winId, act);
            return;
        }
    }

    ComputerUtils::setCursorState(true);
    DevMngIns->mountBlockDevAsync(id, {}, [=](bool ok, DFMMOUNT::DeviceError err, const QString &mpt) {
        bool isOpticalDevice = id.contains(QRegularExpression("/sr[0-9]*$"));
        if (ok || isOpticalDevice) {
            QUrl u = isOpticalDevice ? ComputerUtils::makeBurnUrl(id) : ComputerUtils::makeLocalUrl(mpt);

            if (isOpticalDevice)
                this->waitUDisks2DataReady(id);

            ComputerItemWatcherInstance->insertUrlMapper(id, u);
            if (!shellId.isEmpty())
                ComputerItemWatcherInstance->insertUrlMapper(shellId, QUrl::fromLocalFile(mpt));

            cdTo(id, u, winId, act);
        } else {
            qDebug() << "mount device failed: " << id << err;
            DialogManagerInstance->showErrorDialogWhenMountDeviceFailed(err);
        }
        ComputerUtils::setCursorState();
    });
}

void ComputerController::actionTriggered(DFMEntryFileInfoPointer info, quint64 winId, const QString &actionText, bool triggerFromSidebar)
{
    // if not original supported suffix, publish event to notify subscribers to handle
    QString sfx = info->suffix();
    if (!ComputerUtils::isPresetSuffix(sfx)) {
        ComputerEventCaller::sendContextActionTriggered(winId, info->url(), actionText);
        return;
    }

    if (actionText == ContextMenuActionTrs::trOpenInNewWin())
        actOpenInNewWindow(winId, info);
    else if (actionText == ContextMenuActionTrs::trOpenInNewTab())
        actOpenInNewTab(winId, info);
    else if (actionText == ContextMenuActionTrs::trMount())
        actMount(winId, info);
    else if (actionText == ContextMenuActionTrs::trUnmount())
        actUnmount(info);
    else if (actionText == ContextMenuActionTrs::trRename())
        actRename(winId, info, triggerFromSidebar);
    else if (actionText == ContextMenuActionTrs::trFormat())
        actFormat(winId, info);
    else if (actionText == ContextMenuActionTrs::trSafelyRemove())
        actSafelyRemove(info);
    else if (actionText == ContextMenuActionTrs::trEject())
        actEject(info->url());
    else if (actionText == ContextMenuActionTrs::trProperties())
        actProperties(winId, info);
    else if (actionText == ContextMenuActionTrs::trOpen())
        onOpenItem(0, info->url());
    else if (actionText == ContextMenuActionTrs::trRemove())
        actRemove(info);
    else if (actionText == ContextMenuActionTrs::trLogoutAndClearSavedPasswd())
        actLogoutAndForgetPasswd(info);
    else if (actionText == ContextMenuActionTrs::trErase())
        actErase(info);
}

void ComputerController::actEject(const QUrl &url)
{
    QString id;
    if (url.path().endsWith(SuffixInfo::kBlock)) {
        id = ComputerUtils::getBlockDevIdByUrl(url);
        DevMngIns->detachBlockDev(id, [](bool ok, DFMMOUNT::DeviceError) {
            if (!ok)
                DialogManagerInstance->showErrorDialogWhenUnmountDeviceFailed(DFMMOUNT::DeviceError::kUDisksErrorDeviceBusy);
        });
    } else if (url.path().endsWith(SuffixInfo::kProtocol)) {
        id = ComputerUtils::getProtocolDevIdByUrl(url);
        DevMngIns->unmountProtocolDevAsync(id, {}, [=](bool ok, DFMMOUNT::DeviceError err) {
            if (!ok) {
                qWarning() << "unmount protocol device failed: " << id << err;
                DialogManagerInstance->showErrorDialogWhenUnmountDeviceFailed(err);
            }
        });
    } else {
        qDebug() << url << "is not support " << __FUNCTION__;
    }
}

void ComputerController::actOpenInNewWindow(quint64 winId, DFMEntryFileInfoPointer info)
{
    if (info->order() == EntryFileInfo::kOrderApps) {
        onOpenItem(winId, info->url());
    } else if (info->order() > EntryFileInfo::kOrderCustom) {
        ComputerEventCaller::sendCtrlNOnItem(winId, info->url());
    } else {
        auto target = info->targetUrl();
        if (target.isValid()) {
            if (info->extraProperty(DeviceProperty::kOptical).toBool())
                target = ComputerUtils::makeBurnUrl(ComputerUtils::getBlockDevIdByUrl(info->url()));
            ComputerEventCaller::sendEnterInNewWindow(target);
        } else {
            mountDevice(winId, info, kEnterInNewWindow);
        }
    }
}

void ComputerController::actOpenInNewTab(quint64 winId, DFMEntryFileInfoPointer info)
{
    if (info->order() == EntryFileInfo::kOrderApps) {
        onOpenItem(winId, info->url());
    } else if (info->order() > EntryFileInfo::kOrderCustom) {
        ComputerEventCaller::sendCtrlTOnItem(winId, info->url());
    } else {
        auto target = info->targetUrl();
        if (target.isValid()) {
            if (info->extraProperty(DeviceProperty::kOptical).toBool())
                target = ComputerUtils::makeBurnUrl(ComputerUtils::getBlockDevIdByUrl(info->url()));
            ComputerEventCaller::sendEnterInNewTab(winId, target);
        } else {
            mountDevice(winId, info, kEnterInNewTab);
        }
    }
}

static void onNetworkDeviceMountFinished(bool ok, DFMMOUNT::DeviceError err, const QString &mntPath, quint64 winId, bool enterAfterMounted)
{
    if (ok) {
        if (enterAfterMounted)
            ComputerEventCaller::cdTo(winId, mntPath);
    } else {
        DialogManagerInstance->showErrorDialogWhenMountDeviceFailed(err);
    }
}

void ComputerController::actMount(quint64 winId, DFMEntryFileInfoPointer info, bool enterAfterMounted)
{
    QString sfx = info->suffix();
    if (sfx == SuffixInfo::kStashedProtocol) {
        QString devId = ComputerUtils::getProtocolDevIdByStashedUrl(info->url());
        DevMngIns->mountNetworkDeviceAsync(devId, [devId, enterAfterMounted, winId](bool ok, DFMMOUNT::DeviceError err, const QString &mntPath) {
            if (ok)
                ComputerItemWatcherInstance->insertUrlMapper(devId, QUrl::fromLocalFile(mntPath));
            onNetworkDeviceMountFinished(ok, err, mntPath, winId, enterAfterMounted);
        });
        return;
    } else if (sfx == SuffixInfo::kBlock) {
        mountDevice(0, info, kNone);
        return;
    } else if (sfx == SuffixInfo::kProtocol) {
        return;
    }
}

void ComputerController::actUnmount(DFMEntryFileInfoPointer info)
{
    QString devId;
    if (info->suffix() == SuffixInfo::kBlock) {
        devId = ComputerUtils::getBlockDevIdByUrl(info->url());
        if (info->extraProperty(DeviceProperty::kIsEncrypted).toBool()) {
            QString cleartextId = info->extraProperty(DeviceProperty::kCleartextDevice).toString();
            DevMngIns->unmountBlockDevAsync(cleartextId, {}, [=](bool ok, DFMMOUNT::DeviceError err) {
                if (ok) {
                    DevMngIns->lockBlockDevAsync(devId, {}, [=](bool ok, DFMMOUNT::DeviceError err) {
                        if (!ok)
                            qInfo() << "lock device failed: " << devId << err;
                    });
                } else {
                    qInfo() << "unmount cleartext device failed: " << cleartextId << err;
                    DialogManagerInstance->showErrorDialogWhenUnmountDeviceFailed(err);
                }
            });
        } else {
            DevMngIns->unmountBlockDevAsync(devId, {}, [=](bool ok, DFMMOUNT::DeviceError err) {
                if (!ok) {
                    qInfo() << "unlock device failed: " << devId << err;
                    DialogManagerInstance->showErrorDialogWhenUnmountDeviceFailed(err);
                }
            });
        }
    } else if (info->suffix() == SuffixInfo::kProtocol) {
        devId = ComputerUtils::getProtocolDevIdByUrl(info->url());
        DevMngIns->unmountProtocolDevAsync(devId, {}, [=](bool ok, DFMMOUNT::DeviceError err) {
            if (!ok) {
                qWarning() << "unmount protocol device failed: " << devId << err;
                DialogManagerInstance->showErrorDialogWhenUnmountDeviceFailed(err);
            }
        });
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

    if (info->extraProperty(DeviceProperty::kRemovable).toBool() && info->targetUrl().isValid()) {
        qWarning() << "cannot rename a mounted device! " << __FUNCTION__;
        return;
    }

    if (!triggerFromSidebar)
        Q_EMIT requestRename(winId, info->url());
    else
        SideBarService::service()->triggerItemEdit(winId, info->url());
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
    if (info->suffix() != SuffixInfo::kStashedProtocol)
        return;
    StashMountsUtils::removeStashedMount(info->url());
    Q_EMIT ComputerItemWatcherInstance->removeDevice(info->url());
}

void ComputerController::actProperties(quint64 winId, DFMEntryFileInfoPointer info)
{
    Q_UNUSED(winId);
    if (!info)
        return;

    if (info->order() == EntryFileInfo::EntryOrder::kOrderApps)
        return;

    if (info->suffix() == SuffixInfo::kUserDir) {
        ComputerEventCaller::sendShowPropertyDialog(info->targetUrl());
        return;
    }

    ComputerEventCaller::sendShowPropertyDialog(info->url());
}

void ComputerController::actLogoutAndForgetPasswd(DFMEntryFileInfoPointer info)
{
    // 1. forget passwd
    QString id = ComputerUtils::getProtocolDevIdByUrl(info->url());
    RemotePasswdManagerInstance->clearPasswd(id);

    // 2. unmount
    actUnmount(info);

    // 3. remove stashed entry
    QUrl stashedUrl = ComputerUtils::makeStashedProtocolDevUrl(id);
    StashMountsUtils::removeStashedMount(stashedUrl);
    Q_EMIT ComputerItemWatcherInstance->removeDevice(info->url());
}

void ComputerController::actErase(DFMEntryFileInfoPointer info)
{
    ComputerEventCaller::sendErase(info->extraProperty(DeviceProperty::kDevice).toString());
}

ComputerController::ComputerController(QObject *parent)
    : QObject(parent)
{
}

void ComputerController::waitUDisks2DataReady(const QString &id)
{
    /* this is a workaround, and it's a upstream bug.
     * after mounted, try to obtain the mountpoint through
     * UDisksFilesystem's method right away, empty string is returned.
     * so make short wait here until timeout or the mountpoint is obtained.
     * and this is an upstream issue, the latest of udisks2 seems to have
     * solved this issue already, but the version of ours is too old and costs
     * a lot to make patch.
     * https://github.com/storaged-project/udisks/issues/930
    */
    EntryFileInfo *info { nullptr };
    int maxRetry = 5;
    while (maxRetry > 0) {
        qApp->processEvents();   // if launch without service, this blocks the udisks' event loop.

        if (!info)
            info = new EntryFileInfo(ComputerUtils::makeBlockDevUrl(id));
        if (info->targetUrl().isValid()) {
            break;
        } else {
            QThread::msleep(100);
            info->refresh();
        }
        maxRetry--;
    }
    if (info)
        delete info;
}
