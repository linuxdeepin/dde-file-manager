// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMFILECRUMBCONTROLLER_H
#define DFMFILECRUMBCONTROLLER_H

#include "interfaces/dfmcrumbinterface.h"

DFM_BEGIN_NAMESPACE

class DFMFileCrumbController : public DFMCrumbInterface
{
public:
    explicit DFMFileCrumbController(QObject *parent = nullptr);
    ~DFMFileCrumbController() override;

    bool supportedUrl(DUrl url) override;
    QList<CrumbData> seprateUrl(const DUrl &url) override;

private:
    QString homePath = QString();

    QString getDisplayName(const QString& name) const;
};

DFM_END_NAMESPACE

#endif // DFMFILECRUMBCONTROLLER_H
