// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMFTPCRUMBCONTROLLER_H
#define DFMFTPCRUMBCONTROLLER_H

#include "interfaces/dfmcrumbinterface.h"

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMFtpCrumbController : public DFMCrumbInterface
{
public:
    explicit DFMFtpCrumbController(QObject *parent = nullptr);
    ~DFMFtpCrumbController() override;

    bool supportedUrl(DUrl url) override;
};

DFM_END_NAMESPACE

#endif // DFMSMBCRUMBCONTROLLER_H
