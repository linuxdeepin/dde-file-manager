#include "vaulteventreceiver.h"
#include "utils/vaulthelper.h"

#include "dfm-base/base/urlroute.h"

#include "services/filemanager/computer/computer_defines.h"

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
    dpfInstance.eventDispatcher().subscribe(DSB_FM_NAMESPACE::EventType::kOnOpenItem, this, &VaultEventReceiver::computerOpenItem);
}

void VaultEventReceiver::computerOpenItem(quint64 winId, const QUrl &url)
{
    if (url.path().contains("vault")) {
        VaultHelper::instance()->appendWinID(winId);
        VaultState state = VaultHelper::instance()->state(VaultHelper::instance()->vaultLockPath());
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
