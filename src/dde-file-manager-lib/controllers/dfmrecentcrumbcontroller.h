// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMRECENTCONTROLLER_H
#define DFMRECENTCONTROLLER_H

#include "interfaces/dfmcrumbinterface.h"

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMRecentCrumbController : public DFMCrumbInterface
{
    Q_OBJECT

public:
    explicit DFMRecentCrumbController(QObject *parent = nullptr);

    bool supportedUrl(DUrl url) override;
    QList<CrumbData> seprateUrl(const DUrl &url) override;

private:
    QString homePath = QString();

    QString getDisplayName(const QString& name) const;
};

DFM_END_NAMESPACE

#endif // DFMRECENTCONTROLLER_H
