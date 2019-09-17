#ifndef DFMVAULTCRUMBCONTROLLER_H
#define DFMVAULTCRUMBCONTROLLER_H

#include "interfaces/dfmcrumbinterface.h"

DFM_BEGIN_NAMESPACE

class DFMVaultCrumbController : public DFMCrumbInterface
{
public:
    explicit DFMVaultCrumbController(QObject *parent = nullptr);
    ~DFMVaultCrumbController() override;

    bool supportedUrl(DUrl url) override;
};

DFM_END_NAMESPACE

#endif // DFMVAULTCRUMBCONTROLLER_H
