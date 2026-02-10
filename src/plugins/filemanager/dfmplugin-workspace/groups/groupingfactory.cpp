// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "groupingfactory.h"
#include "nogroupstrategy.h"
#include "namegroupstrategy.h"
#include "sizegroupstrategy.h"
#include "timegroupstrategy.h"
#include "typegroupstrategy.h"
#include "pathgroupstrategy.h"

#include <dfm-base/dfm_log_defines.h>

DPWORKSPACE_USE_NAMESPACE

AbstractGroupStrategy *GroupingFactory::createStrategy(const QString &strategyName, QObject *parent)
{
    fmDebug() << "GroupingFactory: Creating strategy for name:" << strategyName;

    if (strategyName == GroupStrategy::kNoGroup) {
        return new NoGroupStrategy(parent);
    } else if (strategyName == GroupStrategy::kName) {
        return new NameGroupStrategy(parent);
    } else if (strategyName == GroupStrategy::kSize) {
        return new SizeGroupStrategy(parent);
    } else if (strategyName == GroupStrategy::kModifiedTime) {
        return new TimeGroupStrategy(TimeGroupStrategy::kModificationTime, parent);
    } else if (strategyName == GroupStrategy::kCreatedTime) {
        return new TimeGroupStrategy(TimeGroupStrategy::kCreationTime, parent);
    } else if (strategyName == GroupStrategy::kType) {
        return new TypeGroupStrategy(parent);
    } else if (strategyName == GroupStrategy::kCustomPath) {
        return new PathGroupStrategy(parent);
    } else if (strategyName == GroupStrategy::kCustomTime) {
        return new TimeGroupStrategy(TimeGroupStrategy::kCustomTime);
    } else {
        fmWarning() << "GroupingFactory: Unknown strategy name:" << strategyName;
        return nullptr;
    }
}
