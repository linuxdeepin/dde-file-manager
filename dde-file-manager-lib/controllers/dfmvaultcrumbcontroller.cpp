#include "dfmvaultcrumbcontroller.h"

DFM_BEGIN_NAMESPACE

DFMVaultCrumbController::DFMVaultCrumbController(QObject *parent)
    : DFMCrumbInterface(parent)
{

}

DFMVaultCrumbController::~DFMVaultCrumbController()
{

}

bool DFMVaultCrumbController::supportedUrl(DUrl url)
{
    return (url.scheme() == DFMVAULT_SCHEME);
}

DFM_END_NAMESPACE
