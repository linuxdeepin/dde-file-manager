#include "vaulteventreceiver.h"
#include "utils/pathmanager.h"
#include "utils/vaulthelper.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"
#include "dfm-base/interfaces/abstractfilewatcher.h"
#include "dfm-base/interfaces/private/watchercache.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/utils/universalutils.h"

#include <dfm-framework/dpf.h>

Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(Qt::DropAction *)
Q_DECLARE_METATYPE(QString *)

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
    dpfSignalDispatcher->subscribe(GlobalEventType::kHideFilesResult, VaultEventReceiver::instance(), &VaultEventReceiver::handleHideFilesResult);
    dpfSignalDispatcher->installEventFilter(GlobalEventType::kChangeCurrentUrl, this, &VaultEventReceiver::changeUrlEventFilter);

    dpfHookSequence->follow("dfmplugin_utils", "hook_AppendCompress_Prohibit",
                            VaultEventReceiver::instance(), &VaultEventReceiver::handleNotAllowedAppendCompress);
    dpfHookSequence->follow("dfmplugin_sidebar", "hook_Item_DragMoveData",
                            VaultEventReceiver::instance(), &VaultEventReceiver::handleSideBarItemDragMoveData);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_PasteFiles", this, &VaultEventReceiver::handleShortCutPasteFiles);

    dpfHookSequence->follow("dfmplugin_workspace", "hook_Url_FetchPathtoVirtual", this, &VaultEventReceiver::handlePathtoVirtual);

    dpfHookSequence->follow("dfmplugin_detailspace", "hook_Icon_Fetch", this, &VaultEventReceiver::detailViewIcon);
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
            VaultHelper::instance()->creatVaultDialog();
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
            bool ok = dpfHookSequence->run("dfmplugin_utils", "hook_UrlsTransform", QList<QUrl>() << localUrl, &urls);
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
        bool ok = dpfHookSequence->run("dfmplugin_utils", "hook_UrlsTransform", QList<QUrl>() << localUrl, &urls);
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

    if (VaultHelper::isVaultFile(fromUrls.first()) && to.scheme() == "trash") {
        dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kDeleteFiles,
                                     winId,
                                     fromUrls, DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint, nullptr);
        return true;
    }
    return false;
}

void VaultEventReceiver::handleHideFilesResult(const quint64 &winId, const QList<QUrl> &urls, bool ok)
{
    if (ok && !urls.isEmpty()) {
        const QUrl &url = urls.first();
        AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
        if (info) {
            const QUrl &parentUrlVirtual = VaultHelper::instance()->pathToVaultVirtualUrl(
                    info->pathInfo(PathInfo::kPath));
            QSharedPointer<AbstractFileWatcher> watcher = WatcherCache::instance().getCacheWatcher(parentUrlVirtual);
            if (!watcher.isNull()) {
                const QString &hideFilePath = info->pathInfo(PathInfo::kPath) + "/.hidden";
                const QUrl &hideFileUrl = QUrl::fromLocalFile(hideFilePath);
                emit watcher->fileAttributeChanged(hideFileUrl);
            }
        }
    }
}

bool VaultEventReceiver::changeUrlEventFilter(quint64 windowId, const QUrl &url)
{
    if (url.scheme() == VaultHelper::instance()->scheme()) {
        VaultHelper::instance()->appendWinID(windowId);
        const VaultState &state = VaultHelper::instance()->state(PathManager::vaultLockPath());
        if (VaultState::kNotExisted == state) {
            VaultHelper::instance()->creatVaultDialog();
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
