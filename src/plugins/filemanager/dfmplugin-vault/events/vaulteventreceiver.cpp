// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaulteventreceiver.h"
#include "utils/pathmanager.h"
#include "utils/vaulthelper.h"
#include "utils/vaultfilehelper.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>
#include <dfm-base/utils/watchercache.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/fileutils.h>

#include <dfm-framework/dpf.h>

#include <dfm-io/dfmio_utils.h>

Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(Qt::DropAction *)
Q_DECLARE_METATYPE(QString *)
Q_DECLARE_METATYPE(QFileDevice::Permissions)
Q_DECLARE_METATYPE(bool *)

DPF_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultEventReceiver::VaultEventReceiver(QObject *parent)
    : QObject(parent)
{
}

VaultEventReceiver *VaultEventReceiver::instance()
{
    static VaultEventReceiver vaultEventReceiver;
    return &vaultEventReceiver;
}

void VaultEventReceiver::connectEvent()
{
    dpfSignalDispatcher->subscribe(GlobalEventType::kChangeCurrentUrl, VaultEventReceiver::instance(), &VaultEventReceiver::handleCurrentUrlChanged);
    dpfSignalDispatcher->subscribe("dfmplugin_computer", "signal_Operation_OpenItem", this, &VaultEventReceiver::computerOpenItem);
    dpfSignalDispatcher->installEventFilter(GlobalEventType::kChangeCurrentUrl, this, &VaultEventReceiver::changeUrlEventFilter);

    dpfHookSequence->follow("dfmplugin_utils", "hook_AppendCompress_Prohibit", VaultEventReceiver::instance(), &VaultEventReceiver::handleNotAllowedAppendCompress);
    dpfHookSequence->follow("dfmplugin_sidebar", "hook_Item_DragMoveData", VaultEventReceiver::instance(), &VaultEventReceiver::handleSideBarItemDragMoveData);
    dpfHookSequence->follow("dfmplugin_sidebar", "hook_Item_DropData", this, &VaultEventReceiver::fileDropHandleWithAction);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_DragDrop_CheckDragDropAction", VaultFileHelper::instance(), &VaultFileHelper::checkDragDropAction);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_DragDrop_FileDrop", VaultFileHelper::instance(), &VaultFileHelper::handleDropFiles);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_PasteFiles", this, &VaultEventReceiver::handleShortCutPasteFiles);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_Url_FetchPathtoVirtual", this, &VaultEventReceiver::handlePathtoVirtual);
    dpfHookSequence->follow("dfmplugin_detailspace", "hook_Icon_Fetch", this, &VaultEventReceiver::detailViewIcon);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_CutToFile", VaultFileHelper::instance(), &VaultFileHelper::cutFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_CopyFile", VaultFileHelper::instance(), &VaultFileHelper::copyFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_DeleteFile", VaultFileHelper::instance(), &VaultFileHelper::deleteFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin", VaultFileHelper::instance(), &VaultFileHelper::openFileInPlugin);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_RenameFile", VaultFileHelper::instance(), &VaultFileHelper::renameFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_MakeDir", VaultFileHelper::instance(), &VaultFileHelper::makeDir);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_TouchFile", VaultFileHelper::instance(), &VaultFileHelper::touchFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_TouchCustomFile", VaultFileHelper::instance(), &VaultFileHelper::touchCustomFile);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_WriteUrlsToClipboard", VaultFileHelper::instance(), &VaultFileHelper::writeUrlsToClipboard);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_RenameFiles", VaultFileHelper::instance(), &VaultFileHelper::renameFiles);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_RenameFilesAddText", VaultFileHelper::instance(), &VaultFileHelper::renameFilesAddText);
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_SetPermission", VaultFileHelper::instance(), &VaultFileHelper::setPermision);
    dpfHookSequence->follow("dfmplugin_propertydialog", "hook_PermissionView_Ash", this, &VaultEventReceiver::handlePermissionViewAsh);
    dpfHookSequence->follow("dfmplugin_tag", "hook_CanTaged", this, &VaultEventReceiver::handleFileCanTaged);
}

void VaultEventReceiver::computerOpenItem(quint64 winId, const QUrl &url)
{
    if (url.path().contains("vault")) {
        VaultHelper::instance()->appendWinID(winId);
        VaultState state = VaultHelper::instance()->state(PathManager::vaultLockPath());
        switch (state) {
        case VaultState::kUnlocked: {
            VaultHelper::instance()->openWidWindow(winId, VaultHelper::instance()->rootUrl());
        } break;
        case VaultState::kEncrypted: {
            VaultHelper::instance()->unlockVaultDialog();
        } break;
        case VaultState::kNotExisted: {
            VaultHelper::instance()->createVaultDialog();
        } break;
        default:
            break;
        }
    }
}

bool VaultEventReceiver::handleNotAllowedAppendCompress(const QList<QUrl> &fromUrls, const QUrl &toUrl)
{
    QUrl vaultRootUrl = VaultHelper::instance()->sourceRootUrl();
    QString vaultRootPath = vaultRootUrl.path();

    if (!fromUrls.isEmpty()) {
        const QUrl &url = fromUrls.first();
        if (url.isValid()) {
            QUrl localUrl = url;
            QList<QUrl> urls {};
            bool ok = UniversalUtils::urlsTransformToLocal({ localUrl }, &urls);
            if (ok && !urls.isEmpty())
                localUrl = urls.first();

            QString localPath = localUrl.toLocalFile();
            if (localPath.startsWith(vaultRootPath))
                return true;
        }
    }

    if (toUrl.isValid()) {
        QUrl localUrl = toUrl;
        QList<QUrl> urls {};
        bool ok = UniversalUtils::urlsTransformToLocal({ localUrl }, &urls);
        if (ok && !urls.isEmpty())
            localUrl = urls.first();

        QString localPath = localUrl.toLocalFile();
        if (localPath.startsWith(vaultRootPath))
            return true;
    }

    return false;
}

void VaultEventReceiver::handleCurrentUrlChanged(const quint64 &winId, const QUrl &url)
{
    auto window = FMWindowsIns.findWindowById(winId);

    if (url.scheme() == VaultHelper::instance()->scheme() && window)
        VaultHelper::instance()->appendWinID(winId);
    else
        VaultHelper::instance()->removeWinID(winId);
}

bool VaultEventReceiver::handleSideBarItemDragMoveData(const QList<QUrl> &urls, const QUrl &url, Qt::DropAction *action)
{
    // TODO(gongheng): Can think of a better way
    if (url.scheme() != "tag" || urls.isEmpty())
        return false;

    const QUrl &fromUrl = urls.first();
    if (VaultHelper::isVaultFile(fromUrl)) {
        *action = Qt::IgnoreAction;
        return true;
    }
    return false;
}

bool VaultEventReceiver::handleShortCutPasteFiles(const quint64 &winId, const QList<QUrl> &fromUrls, const QUrl &to)
{
    if (fromUrls.isEmpty())
        return false;

    if (VaultHelper::isVaultFile(fromUrls.first()) && FileUtils::isTrashFile(to))
        return true;
    return false;
}

bool VaultEventReceiver::changeUrlEventFilter(quint64 windowId, const QUrl &url)
{
    if (url.scheme() == VaultHelper::instance()->scheme()) {
        VaultHelper::instance()->appendWinID(windowId);
        const VaultState &state = VaultHelper::instance()->state(PathManager::vaultLockPath());
        if (VaultState::kNotExisted == state) {
            VaultHelper::instance()->createVaultDialog();
            return true;
        } else if (VaultState::kEncrypted == state) {
            VaultHelper::instance()->unlockVaultDialog();
            return true;
        } else if (VaultState::kUnlocked == state) {
            return false;
        } else if (VaultState::kNotAvailable == state) {
            DialogManagerInstance->showErrorDialog(tr("Vault"), tr("Vault not available because cryfs not installed!"));
            return true;
        } else {
            return true;
        }
    }
    return false;
}

bool VaultEventReceiver::detailViewIcon(const QUrl &url, QString *iconName)
{
    if (url.scheme() == VaultHelper::instance()->scheme() && UniversalUtils::urlEquals(url, VaultHelper::instance()->rootUrl())) {
        *iconName = "drive-harddisk-encrypted";
        return true;
    }
    return false;
}

bool VaultEventReceiver::handlePathtoVirtual(const QList<QUrl> files, QList<QUrl> *virtualFiles)
{
    if (files.isEmpty())
        return false;
    for (const QUrl &url : files) {
        if (!VaultHelper::isVaultFile(url))
            return false;
        *virtualFiles << VaultHelper::instance()->pathToVaultVirtualUrl(url.path());
    }
    return true;
}

bool VaultEventReceiver::fileDropHandleWithAction(const QList<QUrl> &fromUrls, const QUrl &toUrl, Qt::DropAction *action)
{
    if (VaultHelper::isVaultFile(toUrl)) {
        *action = Qt::CopyAction;
        return true;
    }

    for (const QUrl &url : fromUrls) {
        if (VaultHelper::isVaultFile(url)) {
            *action = Qt::CopyAction;
            return true;
        }
    }
    return false;
}

bool VaultEventReceiver::handlePermissionViewAsh(const QUrl &url, bool *isAsh)
{
    if (!VaultHelper::isVaultFile(url))
        return false;

    *isAsh = true;

    return true;
}

bool VaultEventReceiver::handleFileCanTaged(const QUrl &url, bool *canTag)
{
    if (url.scheme() == VaultHelper::instance()->scheme()) {
        *canTag = false;
        return true;
    }

    return false;
}
