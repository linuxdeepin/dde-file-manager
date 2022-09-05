// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMSMBCRUMBCONTROLLER_H
#define DFMSMBCRUMBCONTROLLER_H

#include "interfaces/dfmcrumbinterface.h"

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMSmbCrumbController : public DFMCrumbInterface
{
public:
    explicit DFMSmbCrumbController(QObject *parent = nullptr);
    ~DFMSmbCrumbController() override;

    bool supportedUrl(DUrl url) override;
};

DFM_END_NAMESPACE

#endif // DFMSMBCRUMBCONTROLLER_H
