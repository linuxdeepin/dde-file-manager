// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GROUPINGFACTORY_H
#define GROUPINGFACTORY_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/interfaces/abstractgroupstrategy.h>
#include <dfm-base/dfm_global_defines.h>

#include <QObject>
#include <QHash>
#include <QString>
#include <QStringList>

DPWORKSPACE_BEGIN_NAMESPACE

using namespace DFMBASE_NAMESPACE;

// A registered external group strategy entry. External plugins (e.g.
// dfmplugin_search) register strategies through the workspace DPF slots; the
// workspace holds them by abstract pointer and never touches business logic.
struct RegisteredGroupStrategy
{
    QString displayName;
    QStringList supportedSchemes;   // empty list = available everywhere
    StrategyFactory factory { nullptr };

    RegisteredGroupStrategy() = default;
    inline explicit RegisteredGroupStrategy(const QVariantMap &map)
        : displayName { map.value(PropertyKey::kGroupStrategyDisplayName).toString() },
          supportedSchemes { map.value(PropertyKey::kGroupStrategySchemes).toStringList() },
          factory { DPF_NAMESPACE::paramGenerator<StrategyFactory>(map.value(PropertyKey::kGroupStrategyFactory)) }
    {
    }
};

class GroupingFactory : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Create a grouping strategy instance by name
     * @param strategyName The name of the strategy to create
     * @param parent The parent object for the created strategy
     * @return A pointer to the created strategy, or nullptr if the name is invalid
     */
    static AbstractGroupStrategy *createStrategy(const QString &strategyName,
                                                 QObject *parent = nullptr);

    // --- External extension point ---
    // The registry is consulted first by createStrategy; built-in strategies
    // are the fallback. Registered at plugin start time, queried at menu/setup
    // time. Main-thread only — no locking.
    static void registerStrategy(const QString &name, const RegisteredGroupStrategy &entry);
    static QList<QPair<QString, QString>> registeredStrategies(const QString &schemeFilter = QString());
    static bool isRegistered(const QString &name);

private:
    GroupingFactory() = delete;  // Prevent instantiation

    static QHash<QString, RegisteredGroupStrategy> &registry();
};

DPWORKSPACE_END_NAMESPACE

#endif   // GROUPINGFACTORY_H
