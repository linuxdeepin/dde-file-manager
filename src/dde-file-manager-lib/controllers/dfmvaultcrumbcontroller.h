// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "interfaces/dfmcrumbinterface.h"

DFM_BEGIN_NAMESPACE

class DFMVaultCrumbController : public DFMCrumbInterface
{
public:
    explicit DFMVaultCrumbController(QObject *parent = nullptr);
    ~DFMVaultCrumbController() override;

    bool supportedUrl(DUrl url) override;
    QList<CrumbData> seprateUrl(const DUrl &url) override;
};

DFM_END_NAMESPACE
