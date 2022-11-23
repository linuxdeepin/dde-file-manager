// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCONFIGMENUHIDDENUNIT_H
#define DCONFIGMENUHIDDENUNIT_H

#include "core/upgradeunit.h"

#include <QList>

namespace dfm_upgrade {
class DConfigMenuHiddenUnit : public UpgradeUnit
{
public:
    DConfigMenuHiddenUnit();

    virtual QString name() override;
    virtual bool initialize(const QMap<QString, QString> &args) override;
    virtual bool upgrade() override;

private:
    static const QMap<QString, QString> &mappedActions();
};
}

#endif   // DCONFIGMENUHIDDENUNIT_H
