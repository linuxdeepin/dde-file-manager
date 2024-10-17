// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "computercontroller.h"
#include "events/computereventcaller.h"
#include "fileentity/appentryfileentity.h"
#include "fileentity/blockentryfileentity.h"
#include "utils/computerutils.h"
#include "utils/remotepasswdmanager.h"
#include "watcher/computeritemwatcher.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-framework/dpf.h>

#include <DMenu>

#include <QDebug>
#include <QApplication>
#include <QProcess>
#include <QTimer>
#include <QThread>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

using namespace dfmplugin_computer;
using namespace GlobalServerDefines;

Q_DECLARE_METATYPE(QString *)
Q_DECLARE_METATYPE(bool *)

ComputerController *ComputerController::instance()
{
    static ComputerController instance;
    return &instance;
}

void ComputerController::onOpenItem(quint64 winId, const QUrl &url)
{
    DFMEntryFileInfoPointer info(new EntryFileInfo(url));

    DFMBASE_USE_NAMESPACE;

    bool isOptical = info->extraProperty(DeviceProperty::kOptical).toBool();
    if (!info->isAccessable() && !isOptical) {
        handleUnAccessableDevCdCall(winId, info);
        return;
    }

    auto target = info->targetUrl();
    if (target.isValid()) {
        if (isOptical)
            target = ComputerUtils::makeBurnUrl(ComputerUtils::getBlockDevIdByUrl(url));
        if (DeviceUtils::isSamba(target) || DeviceUtils::isFtp(target))
            handleNetworkCdCall(winId, info);
        else
            ComputerEventCaller::cdTo(winId, target);
    } else {
        QString suffix = info->nameOf(NameInfoType::kSuffix);
        if (suffix == SuffixInfo::kBlock) {
            mountDevice(winId, info);
        } else if (suffix == SuffixInfo::kAppEntry) {
            QString cmd = info->extraProperty(ExtraPropertyName::kExecuteCommand).toString();
            QProcess::startDetached(cmd);
        } else {
            ComputerEventCaller::sendOpenItem(winId, info->urlOf(UrlInfoType::kUrl));
        }
    }
}

void ComputerController::onMenuRequest(quint64 winId, const QUrl &url, bool triggerFromSidebar)
{
    if (!ComputerUtils::contextMenuEnabled)
        return;

    auto scene = dfmplugin_menu_util::menuSceneCreateScene(ComputerUtils::menuSceneName());
    if (!scene) {
        fmWarning() << "Craete scene for computer failed: " << ComputerUtils::menuSceneName();
        return;
    }

    QVariantHash params {
        { MenuParamKey::kCurrentDir, ComputerUtils::rootUrl() },
        { MenuParamKey::kIsEmptyArea, false },
        { MenuParamKey::kWindowId, winId },
        { MenuParamKey::kSelectFiles, QVariant::fromValue<QList<QUrl>>({ url }) },
    };

    if (!scene->initialize(params)) {
        delete scene;
        scene = nullptr;
        return;
    }

    DMenu m;
    m.setProperty(ContextMenuAction::kActionTriggeredFromSidebar, triggerFromSidebar);
    scene->create(&m);
    scene->updateState(&m);

    auto act = m.exec(QCursor::pos());
    if (act) {
        QList<QUrl> urls { url };
        dpfSignalDispatcher->publish("dfmplugin_computer", "signal_ReportLog_MenuData", act->text(), urls);
        scene->triggered(act);
    }
    delete scene;
    scene = nullptr;
}

void ComputerController::doRename(quint64 winId, const QUrl &url, const QString &name)
{
    Q_UNUSED(winId);

    QString newName(name);
    if (newName.trimmed().isEmpty()) {
        fmInfo() << "empty name is inputed" << name << ", ignore rename action." << url;
        return;
    }

    DFMEntryFileInfoPointer info(new EntryFileInfo(url));
    if (!info)
        return;
    QList<AbstractEntryFileEntity::EntryOrder> typesCanSetAlias { AbstractEntryFileEntity::kOrderSysDiskData,
                                                                  AbstractEntryFileEntity::kOrderSysDiskRoot,
                                                                  AbstractEntryFileEntity::kOrderSysDisks };
    bool shouldSetAlias = typesCanSetAlias.contains(info->order());
    auto rename = [info, url, name]() {
        if (info->nameOf(NameInfoType::kSuffix) != SuffixInfo::kBlock || info->displayName() == name)
            return;
        ComputerUtils::setCursorState(true);
        QString devId = ComputerUtils::getBlockDevIdByUrl(url);   // for now only block devices can be renamed.
        DevMngIns->renameBlockDevAsync(devId, name, {}, [=](bool ok, const DFMMOUNT::OperationErrorInfo &err) {
            ComputerUtils::setCursorState();
            if (!ok)
                fmWarning() << "rename block device failed: " << devId << err.message << err.code;
        });
    };

    if (!shouldSetAlias && info->targetUrl().isValid()) {
        // renaming a mounted device, do unmount first.
        fmDebug() << "rename: do unmount device before rename:" << url;
        DevMngIns->unmountBlockDevAsync(ComputerUtils::getBlockDevIdByUrl(url),
                                        { { OperateParamField::kUnmountWithoutLock, true } },
                                        [=](bool ok, const DFMMOUNT::OperationErrorInfo &err) {
                                            if (ok) {
                                                rename();
                                            } else {
                                                fmInfo() << "rename: cannot unmount device before rename: " << err.message << err.code;
                                                DialogManager::instance()->showErrorDialog(tr("Rename failed"), tr("The device is busy and cannot be renamed now"));
                                            }
                                        });
        return;
    }

    if (shouldSetAlias)
        doSetAlias(info, name);
    else
        rename();
}

void ComputerController::doSetAlias(DFMEntryFileInfoPointer info, const QString &alias)
{
    if (!info)
        return;

    QString uuid = info->extraProperty(DeviceProperty::kUUID).toString();
    auto clearDevInfo = info->extraProperty(BlockAdditionalProperty::kClearBlockProperty).toMap();
    if (!clearDevInfo.isEmpty())
        uuid = clearDevInfo.value(DeviceProperty::kUUID, uuid).toString();

    if (uuid.isEmpty()) {
        fmWarning() << "params exception!" << info->urlOf(UrlInfoType::kUrl);
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
        fmInfo() << "append setting item: " << map;
    }

    Application::genericSetting()->setValue(kAliasGroupName, kAliasItemName, list);

    // update sidebar and computer display
    QString sidebarName = displayAlias.isEmpty() ? info->displayName() : displayAlias;
    QVariantMap map {
        { "Property_Key_DisplayName", sidebarName },
        { "Property_Key_Editable", true }
    };
    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Update", info->urlOf(UrlInfoType::kUrl), map);
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Tab_SetAlias", info->targetUrl(), displayAlias);
    Q_EMIT updateItemAlias(info->urlOf(UrlInfoType::kUrl));
}

void ComputerController::mountDevice(quint64 winId, const DFMEntryFileInfoPointer info, ActionAfterMount act)
{
    if (!info) {
        fmCritical() << "a null info pointer is transfered";
        return;
    }

    bool isEncrypted = info->extraProperty(DeviceProperty::kIsEncrypted).toBool();
    bool isUnlocked = info->extraProperty(DeviceProperty::kCleartextDevice).toString().length() > 1;
    QString shellId = ComputerUtils::getBlockDevIdByUrl(info->urlOf(UrlInfoType::kUrl));
    bool hasFileSystem = info->extraProperty(DeviceProperty::kHasFileSystem).toBool();
    bool isOpticalDrive = info->extraProperty(DeviceProperty::kOpticalDrive).toBool();
    QString driveName = info->extraProperty(DeviceProperty::kDriveModel).toString();

    bool needAskForFormat = info->nameOf(NameInfoType::kSuffix) == SuffixInfo::kBlock
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
            QString passwd;
            bool cancelled = false;
            bool hooked = dpfHookSequence->run("dfmplugin_computer",
                                               "hook_Device_AcquireDevPwd",
                                               info->extraProperty(DeviceProperty::kDevice).toString(),
                                               &passwd,
                                               &cancelled);
            if (cancelled) {
                fmInfo() << "give up unlock device" << info->extraProperty(DeviceProperty::kDevice);
                return;
            }

            if (!hooked) {
                passwd = DialogManagerInstance->askPasswordForLockedDevice(driveName);
                fmInfo() << "password offered by other module.";
            }

            if (passwd.isEmpty()) {
                ComputerUtils::setCursorState();
                return;
            }
            ComputerUtils::setCursorState(true);

            DevMngIns->unlockBlockDevAsync(shellId, passwd, {}, [=](bool ok, const DFMMOUNT::OperationErrorInfo &err, const QString &newId) {
                ComputerUtils::setCursorState();

                if (ok) {
                    auto clearDevUrl = ComputerUtils::makeBlockDevUrl(newId);
                    EntryFileInfo clearDevInfo(clearDevUrl);
                    if (clearDevInfo.extraProperty(DeviceProperty::kFileSystem).toString() == "LVM2_member") {
                        ComputerItemWatcherInstance->removeDevice(ComputerUtils::makeBlockDevUrl(shellId));
                        fmInfo() << "lvm group has been unlockded, remove it." << shellId << newId;
                        return;
                    }

                    this->mountDevice(winId, newId, shellId, act);
                } else {
                    DialogManagerInstance->showErrorDialog(tr("Unlock device failed"), tr("Wrong password"));
                    fmInfo() << "unlock device failed: " << shellId << err.message << err.code;
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

    if (DeviceUtils::isWorkingOpticalDiscId(id)) {
        cdTo(id, ComputerUtils::makeBurnUrl(id), winId, act);
        return;
    }

    const auto &&data = DevProxyMng->queryBlockInfo(id);
    if (data.value(DeviceProperty::kOpticalDrive).toBool() && data.value(DeviceProperty::kOpticalBlank).toBool()) {
        if (!data.value(DeviceProperty::kOpticalWriteSpeed).toStringList().isEmpty()) {   // already load data from xorriso.
            cdTo(id, ComputerUtils::makeBurnUrl(id), winId, act);
            return;
        }
    }

    ComputerUtils::setCursorState(true);
    DevMngIns->mountBlockDevAsync(id, {}, [=](bool ok, const DFMMOUNT::OperationErrorInfo &err, const QString &mpt) {
        if (!ok) {
            if (err.code == DFMMOUNT::DeviceError::kUDisksErrorNotAuthorizedDismissed) {
                ComputerUtils::setCursorState();
                return;
            }
            fmInfo() << "mount device failed: " << id << err.message << err.code;
            DialogManagerInstance->showErrorDialogWhenOperateDeviceFailed(DFMBASE_NAMESPACE::DialogManager::kMount, err);
            return;
        }

        bool isOpticalDevice = id.contains(QRegularExpression("/sr[0-9]*$"));
        if (isOpticalDevice)
            this->waitUDisks2DataReady(id);

        QUrl u = isOpticalDevice ? ComputerUtils::makeBurnUrl(id) : ComputerUtils::makeLocalUrl(mpt);
        ComputerItemWatcherInstance->insertUrlMapper(id, ComputerUtils::makeLocalUrl(mpt));
        if (!shellId.isEmpty())
            ComputerItemWatcherInstance->insertUrlMapper(shellId, QUrl::fromLocalFile(mpt));

        cdTo(id, u, winId, act);
        ComputerUtils::setCursorState();
    });
}

void ComputerController::actEject(const QUrl &url)
{
    QString id;
    if (url.path().endsWith(SuffixInfo::kBlock)) {
        id = ComputerUtils::getBlockDevIdByUrl(url);
        DevMngIns->detachBlockDev(id, [](bool ok, const DFMMOUNT::OperationErrorInfo &err) {
            if (!ok)
                DialogManagerInstance->showErrorDialogWhenOperateDeviceFailed(DFMBASE_NAMESPACE::DialogManager::kUnmount, err);
        });
    } else if (url.path().endsWith(SuffixInfo::kProtocol)) {
        id = ComputerUtils::getProtocolDevIdByUrl(url);
        DevMngIns->unmountProtocolDevAsync(id, {}, [=](bool ok, const DFMMOUNT::OperationErrorInfo &err) {
            if (!ok) {
                fmInfo() << "unmount protocol device failed: " << id << err.message << err.code;
                DialogManagerInstance->showErrorDialogWhenOperateDeviceFailed(DFMBASE_NAMESPACE::DialogManager::kUnmount, err);
            }
        });
    } else {
        fmDebug() << url << "is not support ";
    }
}

void ComputerController::actOpenInNewWindow(quint64 winId, DFMEntryFileInfoPointer info)
{
    if (info->order() == AbstractEntryFileEntity::kOrderApps) {
        onOpenItem(winId, info->urlOf(UrlInfoType::kUrl));
    } else if (info->order() > AbstractEntryFileEntity::kOrderCustom) {
        ComputerEventCaller::sendCtrlNOnItem(winId, info->urlOf(UrlInfoType::kUrl));
    } else {
        auto target = info->targetUrl();
        if (target.isValid()) {
            if (info->extraProperty(DeviceProperty::kOptical).toBool())
                target = ComputerUtils::makeBurnUrl(ComputerUtils::getBlockDevIdByUrl(info->urlOf(UrlInfoType::kUrl)));
            ComputerEventCaller::sendEnterInNewWindow(target);
        } else {
            mountDevice(winId, info, kEnterInNewWindow);
        }
    }
}

void ComputerController::actOpenInNewTab(quint64 winId, DFMEntryFileInfoPointer info)
{
    if (info->order() == AbstractEntryFileEntity::kOrderApps) {
        onOpenItem(winId, info->urlOf(UrlInfoType::kUrl));
    } else if (info->order() > AbstractEntryFileEntity::kOrderCustom) {
        ComputerEventCaller::sendCtrlTOnItem(winId, info->urlOf(UrlInfoType::kUrl));
    } else {
        auto target = info->targetUrl();
        if (target.isValid()) {
            if (info->extraProperty(DeviceProperty::kOptical).toBool())
                target = ComputerUtils::makeBurnUrl(ComputerUtils::getBlockDevIdByUrl(info->urlOf(UrlInfoType::kUrl)));
            ComputerEventCaller::sendEnterInNewTab(winId, target);
        } else {
            mountDevice(winId, info, kEnterInNewTab);
        }
    }
}

void ComputerController::actMount(quint64 winId, DFMEntryFileInfoPointer info, bool enterAfterMounted)
{
    QString sfx = info->nameOf(NameInfoType::kSuffix);
    if (sfx == SuffixInfo::kBlock) {
        mountDevice(0, info, kNone);
        return;
    }
}

void ComputerController::actUnmount(DFMEntryFileInfoPointer info)
{
    QString devId;
    if (info->nameOf(NameInfoType::kSuffix) == SuffixInfo::kBlock) {
        devId = ComputerUtils::getBlockDevIdByUrl(info->urlOf(UrlInfoType::kUrl));
        if (info->extraProperty(DeviceProperty::kIsEncrypted).toBool()) {
            QString cleartextId = info->extraProperty(DeviceProperty::kCleartextDevice).toString();
            DevMngIns->unmountBlockDevAsync(cleartextId, {}, [=](bool ok, const DFMMOUNT::OperationErrorInfo &err) {
                if (ok) {
                    DevMngIns->lockBlockDevAsync(devId, {}, [=](bool ok, const DFMMOUNT::OperationErrorInfo &err) {
                        if (!ok)
                            fmInfo() << "lock device failed: " << devId << err.message << err.code;
                    });
                } else {
                    if (err.code == DFMMOUNT::DeviceError::kUDisksErrorNotAuthorizedDismissed)
                        return;
                    fmInfo() << "unmount cleartext device failed: " << cleartextId << err.message << err.code;
                    DialogManagerInstance->showErrorDialogWhenOperateDeviceFailed(DFMBASE_NAMESPACE::DialogManager::kUnmount, err);
                }
            });
        } else {
            DevMngIns->unmountBlockDevAsync(devId, {}, [=](bool ok, const DFMMOUNT::OperationErrorInfo &err) {
                if (!ok) {
                    if (err.code == DFMMOUNT::DeviceError::kUDisksErrorNotAuthorizedDismissed)
                        return;
                    fmInfo() << "unmount device failed: " << devId << err.message << err.code;
                    DialogManagerInstance->showErrorDialogWhenOperateDeviceFailed(DFMBASE_NAMESPACE::DialogManager::kUnmount, err);
                }
            });
        }
    } else if (info->nameOf(NameInfoType::kSuffix) == SuffixInfo::kProtocol) {
        devId = ComputerUtils::getProtocolDevIdByUrl(info->urlOf(UrlInfoType::kUrl));
        DevMngIns->unmountProtocolDevAsync(devId, {}, [=](bool ok, const DFMMOUNT::OperationErrorInfo &err) {
            if (!ok) {
                fmInfo() << "unmount protocol device failed: " << devId << err.message << err.code;
                DialogManagerInstance->showErrorDialogWhenOperateDeviceFailed(DFMBASE_NAMESPACE::DialogManager::kUnmount, err);
            }
        });
    } else {
        fmDebug() << info->urlOf(UrlInfoType::kUrl) << "is not support ";
    }
}

void ComputerController::actSafelyRemove(DFMEntryFileInfoPointer info)
{
    actEject(info->urlOf(UrlInfoType::kUrl));
}

void ComputerController::actRename(quint64 winId, DFMEntryFileInfoPointer info, bool triggerFromSidebar)
{
    if (!info) {
        fmWarning() << "info is not valid!";
        return;
    }

    auto devUrl = info->urlOf(UrlInfoType::kUrl);
    QPointer<ComputerController> controller(this);

    if (!triggerFromSidebar)
        Q_EMIT controller->requestRename(winId, devUrl);
    else
        QTimer::singleShot(200, [=] { dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_TriggerEdit", winId, devUrl); });
}

void ComputerController::actFormat(quint64 winId, DFMEntryFileInfoPointer info)
{
    if (info->nameOf(NameInfoType::kSuffix) != SuffixInfo::kBlock) {
        fmWarning() << "non block device is not support format" << info->urlOf(UrlInfoType::kUrl);
        return;
    }
    auto url = info->urlOf(UrlInfoType::kUrl);
    QString devDesc = "/dev/" + url.path().remove("." + QString(SuffixInfo::kBlock));
    fmDebug() << "format: device:" << devDesc;

    QString cmd = "dde-device-formatter";
    QStringList args;
    args << "-m=" + QString::number(winId) << devDesc;

    auto callback = [=](bool ok, const DFMMOUNT::OperationErrorInfo &err) {
        if (ok) {
            QProcess::startDetached(cmd, args);
        } else {
            fmInfo() << "format: cannot unmount/lock device before format: " << err.message << err.code;
            DialogManager::instance()->showErrorDialog(tr("Format failed"), tr("The device is busy and cannot be formatted now"));
        }
    };

    const QString &devId = ComputerUtils::getBlockDevIdByUrl(url);
    if (info->targetUrl().isValid()) {
        fmDebug() << "format: do unmount device before format." << url;
        DevMngIns->unmountBlockDevAsync(devId, {}, callback);
        return;
    } else if (info->extraProperty(DeviceProperty::kIsEncrypted).toBool()
               && info->extraProperty(DeviceProperty::kCleartextDevice).toString() != "/") {   // unlocked encrypted device, do lock first.
        // lock the device.
        fmDebug() << "format: do lock device before format." << url;
        DevMngIns->lockBlockDevAsync(devId, {}, callback);
        return;
    }
    QProcess::startDetached(cmd, args);
}

void ComputerController::actProperties(quint64 winId, DFMEntryFileInfoPointer info)
{
    Q_UNUSED(winId);
    if (!info)
        return;

    if (info->order() == AbstractEntryFileEntity::EntryOrder::kOrderApps)
        return;

    if (info->nameOf(NameInfoType::kSuffix) == SuffixInfo::kUserDir) {
        ComputerEventCaller::sendShowPropertyDialog({ info->targetUrl() });
        return;
    }

    ComputerEventCaller::sendShowPropertyDialog({ info->urlOf(UrlInfoType::kUrl) });
}

void ComputerController::actLogoutAndForgetPasswd(DFMEntryFileInfoPointer info)
{
    // 1. forget passwd
    const QString &id = ComputerUtils::getProtocolDevIdByUrl(info->urlOf(UrlInfoType::kUrl));
    QString uri(id);
    if (id.startsWith(DFMBASE_NAMESPACE::Global::Scheme::kSmb)) {
        uri = id;
    } else if (DeviceUtils::isSamba(id)) {
        QString host, share;
        bool ok = DeviceUtils::parseSmbInfo(id, host, share);
        if (!ok) {
            fmWarning() << "computer: cannot parse info, cannot forget item" << id;
            return;
        }

        QUrl temUrl;
        temUrl.setScheme(DFMBASE_NAMESPACE::Global::Scheme::kSmb);
        temUrl.setHost(host);
        temUrl.setPath("/" + share + "/");
        uri = temUrl.toString();
    }

    RemotePasswdManagerInstance->clearPasswd(uri);

    // 2. unmount
    actUnmount(info);

    // 3. remove device enrty
    ComputerItemWatcherInstance->removeDevice(info->urlOf(UrlInfoType::kUrl));
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

void ComputerController::handleUnAccessableDevCdCall(quint64 winId, DFMEntryFileInfoPointer info)
{
    if (!info)
        return;

    fmDebug() << "cannot access device: " << info->urlOf(UrlInfoType::kUrl);
    bool needAskForFormat = info->nameOf(NameInfoType::kSuffix) == SuffixInfo::kBlock
            && !info->extraProperty(DeviceProperty::kHasFileSystem).toBool()
            && !info->extraProperty(DeviceProperty::kIsEncrypted).toBool()
            && !info->extraProperty(DeviceProperty::kOpticalDrive).toBool();
    if (needAskForFormat) {
        if (DialogManagerInstance->askForFormat())
            actFormat(winId, info);
    }
    ComputerUtils::setCursorState();
}

void ComputerController::handleNetworkCdCall(quint64 winId, DFMEntryFileInfoPointer info)
{
    if (!info)
        return;

    auto target = info->targetUrl();
    QString ip, port;
    if (!NetworkUtils::instance()->parseIp(target.path(), ip, port)) {
        fmWarning() << "parse ip address failed: " << target;
        ComputerEventCaller::cdTo(winId, target);
    } else {
        QStringList ports { port };
        static const QStringList &defaultSmbPorts { "445", "139" };
        if (target.scheme() == "smb" && defaultSmbPorts.contains(port))
            ports = defaultSmbPorts;

        ComputerUtils::setCursorState(true);
        NetworkUtils::instance()->doAfterCheckNet(ip, ports, [winId, target, ip](bool ok) {
            ComputerUtils::setCursorState(false);
            if (ok)
                ComputerEventCaller::cdTo(winId, target);
            else
                DialogManagerInstance->showErrorDialog(tr("Mount error"), tr("Cannot access %1").arg(ip));
        });
    }
}
