// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMMASTEREDMEDIACRUMBCONTROLLER_H
#define DFMMASTEREDMEDIACRUMBCONTROLLER_H

#include "dfmcrumbinterface.h"

DFM_BEGIN_NAMESPACE

class DFMMasteredMediaCrumbController : public DFMCrumbInterface
{
    Q_OBJECT
public:
    explicit DFMMasteredMediaCrumbController(QObject *parent = nullptr);
    ~DFMMasteredMediaCrumbController();

    bool supportedUrl(DUrl url) override;
    QList<CrumbData> seprateUrl(const DUrl &url) override;
};

DFM_END_NAMESPACE

#endif // DFMMASTEREDMEDIACRUMBCONTROLLER_H
