// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GROUPINGFACTORY_H
#define GROUPINGFACTORY_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/interfaces/abstractgroupstrategy.h>
#include <dfm-base/dfm_global_defines.h>

#include <QObject>

DPWORKSPACE_BEGIN_NAMESPACE

using namespace DFMBASE_NAMESPACE;

class GroupingFactory : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Create a grouping strategy instance by name
     * @param strategyName The name of the strategy to create
     * @param parent The parent object for the created strategy
     * @return A pointer to the created strategy, or nullptr if the strategy name is invalid
     */
    static AbstractGroupStrategy *createStrategy(const QString &strategyName, QObject *parent = nullptr);

private:
    GroupingFactory() = delete;  // Prevent instantiation
};

DPWORKSPACE_END_NAMESPACE

#endif   // GROUPINGFACTORY_H