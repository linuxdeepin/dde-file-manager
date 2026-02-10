// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
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
    fmDebug() << "Vault: VaultEventReceiver initialized";
}

VaultEventReceiver *VaultEventReceiver::instance()
{
    static VaultEventReceiver vaultEventReceiver;
    return &vaultEventReceiver;
}

void VaultEventReceiver::connectEvent()
{
    fmDebug() << "Vault: Connecting vault event receiver signals and hooks";

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

    fmDebug() << "Vault: All vault event receiver connections established";
}

void VaultEventReceiver::computerOpenItem(quint64 winId, const QUrl &url)
{
    if (url.path().contains("vault")) {
        fmDebug() << "Vault: Processing vault item open request";
        VaultHelper::instance()->appendWinID(winId);
        VaultState state = VaultHelper::instance()->state(PathManager::vaultLockPath());
        fmDebug() << "Vault: Current vault state:" << static_cast<int>(state);

        switch (state) {
        case VaultState::kUnlocked: {
            fmInfo() << "Vault: Opening unlocked vault window";
            VaultHelper::instance()->openWidWindow(winId, VaultHelper::instance()->rootUrl());
        } break;
        case VaultState::kEncrypted: {
            fmInfo() << "Vault: Showing vault unlock dialog";
            VaultHelper::instance()->unlockVaultDialog();
        } break;
        case VaultState::kNotExisted: {
            fmInfo() << "Vault: Showing vault creation dialog";
            VaultHelper::instance()->createVaultDialog();
        } break;
        default:
            fmWarning() << "Vault: Unknown vault state:" << static_cast<int>(state);
            break;
        }
    }
}

bool VaultEventReceiver::handleNotAllowedAppendCompress(const QList<QUrl> &fromUrls, const QUrl &toUrl)
{
    QUrl vaultRootUrl = VaultHelper::instance()->sourceRootUrl();
    QString vaultRootPath = vaultRootUrl.path();
    fmDebug() << "Vault: Vault root path:" << vaultRootPath;

    if (!fromUrls.isEmpty()) {
        const QUrl &url = fromUrls.first();
        if (url.isValid()) {
            QUrl localUrl = url;
            QList<QUrl> urls {};
            bool ok = UniversalUtils::urlsTransformToLocal({ localUrl }, &urls);
            if (ok && !urls.isEmpty())
                localUrl = urls.first();

            QString localPath = localUrl.toLocalFile();
            if (localPath.startsWith(vaultRootPath)) {
                fmDebug() << "Vault: Prohibiting append compress - from URL is in vault";
                return true;
            }
        }
    }

    if (toUrl.isValid()) {
        QUrl localUrl = toUrl;
        QList<QUrl> urls {};
        bool ok = UniversalUtils::urlsTransformToLocal({ localUrl }, &urls);
        if (ok && !urls.isEmpty())
            localUrl = urls.first();

        QString localPath = localUrl.toLocalFile();
        if (localPath.startsWith(vaultRootPath)) {
            fmDebug() << "Vault: Prohibiting append compress - to URL is in vault";
            return true;
        }
    }

    fmDebug() << "Vault: Append compress allowed";
    return false;
}

void VaultEventReceiver::handleCurrentUrlChanged(const quint64 &winId, const QUrl &url)
{
    fmDebug() << "Vault: Handling URL change event - windowId:" << winId << "url:" << url.toString();
    auto window = FMWindowsIns.findWindowById(winId);

    if (url.scheme() == VaultHelper::instance()->scheme() && window) {
        fmDebug() << "Vault: Adding window to vault window list";
        VaultHelper::instance()->appendWinID(winId);
    } else {
        fmDebug() << "Vault: Removing window from vault window list";
        VaultHelper::instance()->removeWinID(winId);
    }
}

bool VaultEventReceiver::handleSideBarItemDragMoveData(const QList<QUrl> &urls, const QUrl &url, Qt::DropAction *action)
{
    // TODO(gongheng): Can think of a better way
    if (url.scheme() != "tag" || urls.isEmpty()) {
        fmDebug() << "Vault: Ignoring non-tag scheme or empty URL list";
        return false;
    }

    const QUrl &fromUrl = urls.first();
    if (VaultHelper::isVaultFile(fromUrl)) {
        fmDebug() << "Vault: Setting drag action to ignore for vault file";
        *action = Qt::IgnoreAction;
        return true;
    }
    return false;
}

bool VaultEventReceiver::handleShortCutPasteFiles(const quint64 &winId, const QList<QUrl> &fromUrls, const QUrl &to)
{
    if (fromUrls.isEmpty()) {
        fmDebug() << "Vault: Empty from URLs list";
        return false;
    }

    if (VaultHelper::isVaultFile(fromUrls.first()) && FileUtils::isTrashFile(to)) {
        fmDebug() << "Vault: Allowing paste from vault to trash";
        return true;
    }

    return false;
}

bool VaultEventReceiver::changeUrlEventFilter(quint64 windowId, const QUrl &url)
{
    if (url.scheme() == VaultHelper::instance()->scheme()) {
        fmDebug() << "Vault: Processing vault URL change";
        VaultHelper::instance()->appendWinID(windowId);
        const VaultState &state = VaultHelper::instance()->state(PathManager::vaultLockPath(), false);
        fmDebug() << "Vault: Current vault state:" << static_cast<int>(state);

        if (VaultState::kNotExisted == state) {
            fmDebug() << "Vault: Showing vault creation dialog";
            VaultHelper::instance()->createVaultDialog();
            return true;
        } else if (VaultState::kEncrypted == state) {
            fmDebug() << "Vault: Showing vault unlock dialog";
            VaultHelper::instance()->unlockVaultDialog();
            return true;
        } else if (VaultState::kUnlocked == state) {
            fmDebug() << "Vault: Vault is unlocked, allowing URL change";
            return false;
        } else if (VaultState::kNotAvailable == state) {
            fmWarning() << "Vault: Vault not available - cryfs not installed";
            DialogManagerInstance->showErrorDialog(tr("Vault"), tr("Vault not available because cryfs not installed!"));
            return true;
        } else {
            fmDebug() << "Vault: Unknown vault state, blocking URL change";
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
    fmDebug() << "Vault: Handling path to virtual conversion, file count:" << files.size();
    if (files.isEmpty()) {
        fmDebug() << "Vault: Empty files list";
        return false;
    }

    for (const QUrl &url : files) {
        if (!VaultHelper::isVaultFile(url)) {
            fmDebug() << "Vault: Non-vault file found, aborting conversion";
            return false;
        }
        QUrl virtualUrl = VaultHelper::instance()->pathToVaultVirtualUrl(url.path());
        *virtualFiles << virtualUrl;
        fmDebug() << "Vault: Converted" << url.toString() << "to virtual URL:" << virtualUrl.toString();
    }

    return true;
}

bool VaultEventReceiver::fileDropHandleWithAction(const QList<QUrl> &fromUrls, const QUrl &toUrl, Qt::DropAction *action)
{
    if (VaultHelper::isVaultFile(toUrl)) {
        fmDebug() << "Vault: Setting drop action to copy for vault target";
        *action = Qt::CopyAction;
        return true;
    }

    for (const QUrl &url : fromUrls) {
        if (VaultHelper::isVaultFile(url)) {
            fmDebug() << "Vault: Setting drop action to copy for vault source file";
            *action = Qt::CopyAction;
            return true;
        }
    }
    return false;
}

bool VaultEventReceiver::handlePermissionViewAsh(const QUrl &url, bool *isAsh)
{
    if (!VaultHelper::isVaultFile(url)) {
        fmDebug() << "Vault: Not a vault file, using default permission view";
        return false;
    }

    *isAsh = true;
    fmDebug() << "Vault: Setting permission view to ash for vault file";
    return true;
}

bool VaultEventReceiver::handleFileCanTaged(const QUrl &url, bool *canTag)
{
    if (url.scheme() == VaultHelper::instance()->scheme()) {
        *canTag = false;
        fmDebug() << "Vault: Vault files cannot be tagged";
        return true;
    }

    return false;
}
