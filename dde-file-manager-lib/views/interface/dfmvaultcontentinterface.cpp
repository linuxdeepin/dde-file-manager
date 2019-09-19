#include "dfmvaultcontentinterface.h"

DFM_BEGIN_NAMESPACE

DFMVaultContentInterface::DFMVaultContentInterface(QWidget *parent) : QWidget(parent)
{

}

QPair<DUrl, bool> DFMVaultContentInterface::requireRedirect(VaultController::VaultState state)
{
    Q_UNUSED(state);
    return {DUrl(), false};
}

DFM_END_NAMESPACE
