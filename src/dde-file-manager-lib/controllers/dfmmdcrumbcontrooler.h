// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMMDCRUMBCONTROLLER_H
#define DFMMDCRUMBCONTROLLER_H

#include "interfaces/dfmcrumbinterface.h"

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

// DFM Merged Desktop (dfmmd://) scheme, crumb support
class DFMMDCrumbControoler : public DFMCrumbInterface
{
public:
    explicit DFMMDCrumbControoler(QObject *parent = nullptr);
    ~DFMMDCrumbControoler() override;

    bool supportedUrl(DUrl url) override;
};

DFM_END_NAMESPACE

#endif // DFMMDCRUMBCONTROLLER_H
