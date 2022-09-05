// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMMTPCRUMBCONTROLLER_H
#define DFMMTPCRUMBCONTROLLER_H

#include "interfaces/dfmcrumbinterface.h"

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMMtpCrumbController : public DFMCrumbInterface
{
public:
    explicit DFMMtpCrumbController(QObject *parent = nullptr);
    ~DFMMtpCrumbController() override;

    bool supportedUrl(DUrl url) override;
};

DFM_END_NAMESPACE

#endif // DFMMTPCRUMBCONTROLLER_H
