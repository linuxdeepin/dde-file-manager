#include "dfmmasteredmediacrumbcontroller.h"

DFM_BEGIN_NAMESPACE

DFMMasteredMediaCrumbController::DFMMasteredMediaCrumbController(QObject *parent):DFMCrumbInterface(parent)
{

}
DFMMasteredMediaCrumbController::~DFMMasteredMediaCrumbController()
{
}

void DFMMasteredMediaCrumbController::processAction(ActionType type)
{
    DFMCrumbInterface::processAction(type);
}
void DFMMasteredMediaCrumbController::crumbUrlChangedBehavior(const DUrl url)
{
    DFMCrumbInterface::crumbUrlChangedBehavior(url);
}
bool DFMMasteredMediaCrumbController::supportedUrl(DUrl url)
{
    return url.scheme()==BURN_SCHEME;
}

DFM_END_NAMESPACE
