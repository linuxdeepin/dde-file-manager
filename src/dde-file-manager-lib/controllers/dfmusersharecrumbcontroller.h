// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMUSERSHARECRUMBCONTROLLER_H
#define DFMUSERSHARECRUMBCONTROLLER_H

#include "interfaces/dfmcrumbinterface.h"

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMUserShareCrumbController : public DFMCrumbInterface
{
public:
    explicit DFMUserShareCrumbController(QObject *parent = nullptr);
    ~DFMUserShareCrumbController() override;

    bool supportedUrl(DUrl url) override;
    QList<CrumbData> seprateUrl(const DUrl &url) override;
};

DFM_END_NAMESPACE

#endif // DFMUSERSHARECRUMBCONTROLLER_H
