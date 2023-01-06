// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMTRASHCRUMBCONTROLLER_H
#define DFMTRASHCRUMBCONTROLLER_H

#include "interfaces/dfmcrumbinterface.h"

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMTrashCrumbController : public DFMCrumbInterface
{
public:
    explicit DFMTrashCrumbController(QObject *parent = nullptr);
    ~DFMTrashCrumbController();

    bool supportedUrl(DUrl url) override;
};

DFM_END_NAMESPACE

#endif // DFMTRASHCRUMBCONTROLLER_H
