// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMTAGCRUMBCONTROLLER_H
#define DFMTAGCRUMBCONTROLLER_H

#include "interfaces/dfmcrumbinterface.h"

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMTagCrumbController : public DFMCrumbInterface
{
public:
    explicit DFMTagCrumbController(QObject *parent = nullptr);
    ~DFMTagCrumbController();

    bool supportedUrl(DUrl url) override;
    QList<CrumbData> seprateUrl(const DUrl &url) override;
};

DFM_END_NAMESPACE

#endif // DFMTAGCRUMBCONTROLLER_H
