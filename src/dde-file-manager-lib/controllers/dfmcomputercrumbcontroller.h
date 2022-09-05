// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMCOMPUTERCRUMBCONTROLLER_H
#define DFMCOMPUTERCRUMBCONTROLLER_H

#include "interfaces/dfmcrumbinterface.h"

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMComputerCrumbController : public DFMCrumbInterface
{
public:
    explicit DFMComputerCrumbController(QObject *parent = nullptr);
    ~DFMComputerCrumbController() override;

    bool supportedUrl(DUrl url) override;
    QList<CrumbData> seprateUrl(const DUrl &url) override;
};

DFM_END_NAMESPACE

#endif // DFMCOMPUTERCRUMBCONTROLLER_H
