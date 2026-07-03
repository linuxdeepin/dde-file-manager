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

QHash<QString, RegisteredGroupStrategy> &GroupingFactory::registry()
{
    static QHash<QString, RegisteredGroupStrategy> s_registry;
    return s_registry;
}

void GroupingFactory::registerStrategy(const QString &name, const RegisteredGroupStrategy &entry)
{
    if (name.isEmpty() || !entry.factory) {
        fmWarning() << "GroupingFactory: refusing to register invalid strategy:" << name;
        return;
    }
    registry().insert(name, entry);
    fmInfo() << "GroupingFactory: registered external strategy:" << name
             << "schemes:" << entry.supportedSchemes;
}

QList<QPair<QString, QString>> GroupingFactory::registeredStrategies(const QString &schemeFilter)
{
    QList<QPair<QString, QString>> result;
    for (auto it = registry().constBegin(); it != registry().constEnd(); ++it) {
        if (!schemeFilter.isEmpty()
            && !it.value().supportedSchemes.isEmpty()
            && !it.value().supportedSchemes.contains(schemeFilter)) {
            continue;
        }
        result.append({ it.key(), it.value().displayName });
    }
    return result;
}

bool GroupingFactory::isRegistered(const QString &name)
{
    return registry().contains(name);
}

AbstractGroupStrategy *GroupingFactory::createStrategy(const QString &strategyName,
                                                       QObject *parent)
{
    fmDebug() << "GroupingFactory: Creating strategy for name:" << strategyName;

    // External extension point first — registered strategies win over built-ins
    // of the same name (allows overrides; harmless because we register at start
    // time and never collide with built-in names in practice).
    auto it = registry().constFind(strategyName);
    if (it != registry().constEnd() && it.value().factory) {
        AbstractGroupStrategy *s = it.value().factory(parent);
        if (s) {
            s->setParent(parent);
            return s;
        }
        fmWarning() << "GroupingFactory: external factory returned null for:" << strategyName;
    }

    // Built-in fallback (hardcoded if/else — YAGNI on refactoring to registration)
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
