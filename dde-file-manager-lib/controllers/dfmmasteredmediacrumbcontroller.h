#ifndef DFMMASTEREDMEDIACRUMBCONTROLLER_H
#define DFMMASTEREDMEDIACRUMBCONTROLLER_H

#include "dfmcrumbinterface.h"

DFM_BEGIN_NAMESPACE

class DFMMasteredMediaCrumbController : public DFMCrumbInterface
{
    Q_OBJECT
public:
    DFMMasteredMediaCrumbController(QObject *parent = 0);
    ~DFMMasteredMediaCrumbController();

    void processAction(ActionType type) override;
    void crumbUrlChangedBehavior(const DUrl url);
    bool supportedUrl(DUrl url) override;
};

DFM_END_NAMESPACE

#endif // DFMMASTEREDMEDIACRUMBCONTROLLER_H
