// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMSFTPCRUMBCONTROLLER_H
#define DFMSFTPCRUMBCONTROLLER_H

#include "interfaces/dfmcrumbinterface.h"

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMSftpCrumbController : public DFMCrumbInterface
{
public:
    explicit DFMSftpCrumbController(QObject *parent = nullptr);
    ~DFMSftpCrumbController() override;

    bool supportedUrl(DUrl url) override;
};

DFM_END_NAMESPACE

#endif // DFMSMBCRUMBCONTROLLER_H
