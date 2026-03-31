// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTENTINDEXUPGRADEUNIT_H
#define CONTENTINDEXUPGRADEUNIT_H

#include "core/upgradeunit.h"

namespace dfm_upgrade {

class ContentIndexUpgradeUnit : public UpgradeUnit
{
public:
    ContentIndexUpgradeUnit();

    QString name() override;
    bool initialize(const QMap<QString, QString> &args) override;
    bool upgrade() override;
    void completed() override;

private:
    bool canMoveOldIndexDirectory() const;

private:
    QString oldIndexDirPath;
    QString newIndexDirPath;
    QString oldStatusFilePath;
    QString newStatusFilePath;
};

}

#endif   // CONTENTINDEXUPGRADEUNIT_H
