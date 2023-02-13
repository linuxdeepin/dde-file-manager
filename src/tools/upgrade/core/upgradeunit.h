// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UPGRADEUNIT_H
#define UPGRADEUNIT_H

#include <QMap>

namespace dfm_upgrade {

class UpgradeUnit
{
public:
    UpgradeUnit();
    virtual ~UpgradeUnit();
    virtual QString name() = 0;
    virtual bool initialize(const QMap<QString, QString> &args) = 0;
    virtual bool upgrade() = 0;
    virtual void completed();
};

}

#endif // UPGRADEUNIT_H
