// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "upgradeunit.h"

#include <QStandardPaths>

using namespace dfm_upgrade;

UpgradeUnit::UpgradeUnit()
{
    configurationPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first() + "/deepin/dde-file-manager/dde-file-manager.json";
    backupDirPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first() + "/deepin/dde-file-manager/old";
}

UpgradeUnit::~UpgradeUnit()
{

}

void UpgradeUnit::completed()
{

}
