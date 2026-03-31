// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UNITLIST_H
#define UNITLIST_H

#include "core/upgradeunit.h"

// units
#include "headerunit.h"
#include "dconfigupgradeunit.h"
#include "bookmarkupgradeunit.h"
#include "tagdbupgradeunit.h"
#include "smbvirtualentryupgradeunit.h"
#include "vaultupgradeunit.h"
#include "desktoporganizeupgradeunit.h"
#include "appattributeupgradeunit.h"
#include "contentindexupgradeunit.h"
// units end

#include <QList>
#include <QSharedPointer>

#define RegUnit(unit) \
    QSharedPointer<UpgradeUnit>(new unit)

namespace dfm_upgrade {

inline QList<QSharedPointer<UpgradeUnit>> createUnits()
{
    return QList<QSharedPointer<UpgradeUnit>> {
        RegUnit(dfm_upgrade::HeaderUnit),
        RegUnit(dfm_upgrade::DConfigUpgradeUnit),
        RegUnit(dfm_upgrade::BookMarkUpgradeUnit),
        RegUnit(dfm_upgrade::TagDbUpgradeUnit),
        RegUnit(dfm_upgrade::SmbVirtualEntryUpgradeUnit),
        RegUnit(dfm_upgrade::VaultUpgradeUnit),
        RegUnit(dfm_upgrade::DesktopOrganizeUpgradeUnit),
        RegUnit(dfm_upgrade::AppAttributeUpgradeUnit),
        RegUnit(dfm_upgrade::ContentIndexUpgradeUnit)
    };
}

}

#endif   // UNITLIST_H
