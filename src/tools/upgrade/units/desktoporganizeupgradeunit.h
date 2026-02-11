// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DESKTOPORGANIZEUPGRADEUNIT_H
#define DESKTOPORGANIZEUPGRADEUNIT_H

#include "core/upgradeunit.h"

#include <dfm-base/dfm_log_defines.h>

namespace dfm_upgrade {
class DesktopOrganizeUpgradeUnit : public UpgradeUnit
{
public:
    DesktopOrganizeUpgradeUnit();

    virtual QString name() override;
    virtual bool initialize(const QMap<QString, QString> &args) override;
    virtual bool upgrade() override;

private:
    QString cfgPath;
};
}

#endif   // DESKTOPORGANIZEUPGRADEUNIT_H
