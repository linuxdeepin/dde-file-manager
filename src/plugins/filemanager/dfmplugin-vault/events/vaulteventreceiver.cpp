#include "vaulteventreceiver.h"
#include "utils/pathmanager.h"
#include "utils/vaulthelper.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

#include <dfm-framework/dpf.h>

Q_DECLARE_METATYPE(QList<QUrl> *)

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

    dpfHookSequence->follow("dfmplugin_utils", "hook_AppendCompress_Prohibit",
                            VaultEventReceiver::instance(), &VaultEventReceiver::handleNotAllowedAppendCompress);
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
