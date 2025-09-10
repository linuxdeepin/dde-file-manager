// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "groupingfactory.h"
#include "nogroupstrategy.h"
#include "namegroupstrategy.h"
#include "sizegroupstrategy.h"
#include "timegroupstrategy.h"
#include "typegroupstrategy.h"

#include <dfm-base/dfm_log_defines.h>

DPWORKSPACE_USE_NAMESPACE

AbstractGroupStrategy *GroupingFactory::createStrategy(const QString &strategyName, QObject *parent)
{
    fmDebug() << "GroupingFactory: Creating strategy for name:" << strategyName;

    if (strategyName == GroupStrategty::kNoGroup) {
        return new NoGroupStrategy(parent);
    } else if (strategyName == GroupStrategty::kName) {
        return new NameGroupStrategy(parent);
    } else if (strategyName == GroupStrategty::kSize) {
        return new SizeGroupStrategy(parent);
    } else if (strategyName == GroupStrategty::kModifiedTime) {
        return new TimeGroupStrategy(TimeGroupStrategy::ModificationTime, parent);
    } else if (strategyName == GroupStrategty::kCreatedTime) {
        return new TimeGroupStrategy(TimeGroupStrategy::CreationTime, parent);
    } else if (strategyName == GroupStrategty::kType) {
        return new TypeGroupStrategy(parent);
    } else {
        fmWarning() << "GroupingFactory: Unknown strategy name:" << strategyName;
        return nullptr;
    }
}