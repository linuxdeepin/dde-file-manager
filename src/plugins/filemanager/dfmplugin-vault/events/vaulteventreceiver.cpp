#include "vaulteventreceiver.h"
#include "utils/pathmanager.h"
#include "utils/vaulthelper.h"

#include "dfm-base/base/urlroute.h"

#include "services/filemanager/computer/computer_defines.h"
#include "services/common/delegate/delegateservice.h"

#include <dfm-framework/dpf.h>
#include <dfm-framework/framework.h>

DPF_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPVAULT_USE_NAMESPACE
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
    dpfSignalDispatcher->subscribe(DSB_FM_NAMESPACE::EventType::kOnOpenItem, this, &VaultEventReceiver::computerOpenItem);
    dpfHookSequence->follow("dfmplugin_utils", "hook_NotAllowdAppendCompress",
                            VaultEventReceiver::instance(), &VaultEventReceiver::handleNotAllowedAppendCompress);
    dpfSignalDispatcher->subscribe("dfmplugin_workspace", "signal_EnterFileView", VaultEventReceiver::instance(), &VaultEventReceiver::EnterFileView);
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
            QUrl localUrl = delegateServIns->urlTransform(url);
            QString localPath = localUrl.toLocalFile();
            if (localPath.startsWith(vaultRootPath))
                return true;
        }
    }

    if (toUrl.isValid()) {
        QUrl localUrl = delegateServIns->urlTransform(toUrl);
        QString localPath = localUrl.toLocalFile();
        if (localPath.startsWith(vaultRootPath))
            return true;
    }

    return false;
}

void VaultEventReceiver::EnterFileView(const quint64 &winId, const QUrl &url)
{
    if (url.scheme() == VaultHelper::instance()->scheme())
        VaultHelper::instance()->appendWinID(winId);
    else
        VaultHelper::instance()->removeWinID(winId);
}
