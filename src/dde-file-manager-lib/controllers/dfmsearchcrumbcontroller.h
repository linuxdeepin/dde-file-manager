// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMSEARCHCRUMBCONTROLLER_H
#define DFMSEARCHCRUMBCONTROLLER_H

#include "interfaces/dfmcrumbinterface.h"

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMSearchCrumbController : public DFMCrumbInterface
{
public:
    explicit DFMSearchCrumbController(QObject *parent = nullptr);
    ~DFMSearchCrumbController();

    void processAction(ActionType type) override;
    void crumbUrlChangedBehavior(const DUrl url);
    bool supportedUrl(DUrl url) override;
};

DFM_END_NAMESPACE

#endif // DFMSEARCHCRUMBCONTROLLER_H
