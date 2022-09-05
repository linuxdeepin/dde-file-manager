// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMAVFSCRUMBCONTROLLER_H
#define DFMAVFSCRUMBCONTROLLER_H

#include "controllers/dfmfilecrumbcontroller.h"

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMAvfsCrumbController : public DFMFileCrumbController
{
public:
    explicit DFMAvfsCrumbController(QObject *parent = nullptr);
    ~DFMAvfsCrumbController();

    bool supportedUrl(DUrl url) override;
};

DFM_END_NAMESPACE

#endif // DFMAVFSCRUMBCONTROLLER_H
