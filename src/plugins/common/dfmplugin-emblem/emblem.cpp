// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "emblem.h"
#include "events/emblemeventrecevier.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

DPEMBLEM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

void Emblem::initialize()
{
    EmblemEventRecevier::instance()->initializeConnections();
}

bool Emblem::start()
{
    QString err;
    auto ret = DConfigManager::instance()->addConfig(kConfigPath, &err);
    if (!ret) {
        qWarning() << "create dconfig failed: " << err;
    } else {
        bool hideEmblems = DConfigManager::instance()->value(kConfigPath, kHideSystemEmblems, false).toBool();
        if (hideEmblems)
           qWarning() << "The system file emblems be hidden by GroupPolicy!";
    }

    return true;
}
