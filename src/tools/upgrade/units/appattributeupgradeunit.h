// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPATTRIBUTEUPGRADEUNIT_H
#define APPATTRIBUTEUPGRADEUNIT_H

#include "core/upgradeunit.h"

#include <QJsonObject>

namespace dfm_upgrade {

class AppAttributeUpgradeUnit : public UpgradeUnit
{
public:
    AppAttributeUpgradeUnit();

    QString name() override;
    bool initialize(const QMap<QString, QString> &args) override;
    bool upgrade() override;
    void completed() override;

private:
    int transIconSizeLevel(int oldIconSizeLevel) const;
    bool backupAppAttribute() const;
    bool writeConfigFile() const;

private:
    QJsonObject configObject;
    int oldIconSizeLevel;
};

}

#endif
