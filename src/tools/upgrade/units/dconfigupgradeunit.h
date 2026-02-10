// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCONFIGUPGRADEUNIT_H
#define DCONFIGUPGRADEUNIT_H

#include "core/upgradeunit.h"

#include <QList>

namespace dfm_upgrade {
class DConfigUpgradeUnit : public UpgradeUnit
{
public:
    DConfigUpgradeUnit();

    virtual QString name() override;
    virtual bool initialize(const QMap<QString, QString> &args) override;
    virtual bool upgrade() override;

private:
    static const QMap<QString, QString> &mappedActions();
    bool upgradeMenuConfigs();
    bool upgradeSmbConfigs();
    bool upgradeRecentConfigs();
    bool upgradeSearchConfigs();
    void clearDiskHidden();
    void addOldGenericSettings();
    bool checkOldGeneric(const QString &key);

private:
    QStringList oldGenericSettings;
};
}

#endif   // DCONFIGUPGRADEUNIT_H
