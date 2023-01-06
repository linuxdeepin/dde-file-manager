// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMBOOKMARKCRUMBCONTROLLER_H
#define DFMBOOKMARKCRUMBCONTROLLER_H

#include "interfaces/dfmcrumbinterface.h"

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMBookmarkCrumbController : public DFMCrumbInterface
{
public:
    explicit DFMBookmarkCrumbController(QObject *parent = nullptr);
    ~DFMBookmarkCrumbController() override;

    bool supportedUrl(DUrl url) override;
    QList<CrumbData> seprateUrl(const DUrl &url) override;
};

DFM_END_NAMESPACE

#endif // DFMBOOKMARKCRUMBCONTROLLER_H
