// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HEADERUNIT_H
#define HEADERUNIT_H

#include "core/upgradeunit.h"

#include <QElapsedTimer>

namespace dfm_upgrade {

class HeaderUnit : public UpgradeUnit
{
public:
    HeaderUnit();
    QString name() override;
    bool initialize(const QMap<QString, QString> &args) override;
    bool upgrade() override;
    void completed() override;

protected:
    QElapsedTimer time;
};

}

#endif   // HEADERUNIT_H
