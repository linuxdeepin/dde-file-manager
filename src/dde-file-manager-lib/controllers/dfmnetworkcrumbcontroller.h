// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMNETWORKCRUMBCONTROLLER_H
#define DFMNETWORKCRUMBCONTROLLER_H

#include "interfaces/dfmcrumbinterface.h"

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMNetworkCrumbController : public DFMCrumbInterface
{
public:
    explicit DFMNetworkCrumbController(QObject *parent = nullptr);
    ~DFMNetworkCrumbController() override;

    bool supportedUrl(DUrl url) override;
    QList<CrumbData> seprateUrl(const DUrl &url) override;
};

DFM_END_NAMESPACE

#endif // DFMNETWORKCRUMBCONTROLLER_H
