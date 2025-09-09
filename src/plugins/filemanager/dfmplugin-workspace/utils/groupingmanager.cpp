// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "groupingmanager.h"
#include "workspacehelper.h"

// Include strategy implementations
#include "groups/nogroupstrategy.h"
#include "groups/typegroupstrategy.h"
#include "groups/timegroupstrategy.h"
#include "groups/namegroupstrategy.h"
#include "groups/sizegroupstrategy.h"

#include <dfm-base/base/schemefactory.h>

DPWORKSPACE_BEGIN_NAMESPACE

using namespace DFMBASE_NAMESPACE;

// Static member initialization
QStringList GroupingManager::s_supportedStrategies;

GroupingManager::GroupingManager(const QUrl &dirUrl, QObject *parent)
    : QObject(parent)
    , m_dirUrl(dirUrl)
    , m_currentStrategy("NoGroupStrategy")
    , m_groupingEnabled(false)
    , m_groupingOrder(Qt::AscendingOrder)
{
    fmDebug() << "GroupingManager created for URL:" << m_dirUrl.toString();
}

GroupingManager::~GroupingManager()
{
    fmDebug() << "GroupingManager destroyed for URL:" << m_dirUrl.toString();
}

void GroupingManager::setGroupingStrategy(const QString &strategyName)
{
    if (!validateStrategy(strategyName)) {
        fmWarning() << "Invalid grouping strategy:" << strategyName << "for URL:" << m_dirUrl.toString();
        return;
    }

    if (m_currentStrategy == strategyName) {
        fmDebug() << "Grouping strategy unchanged, skipping for URL:" << m_dirUrl.toString() << "strategy:" << strategyName;
        return;
    }

    fmInfo() << "Setting grouping strategy:" << strategyName << "for URL:" << m_dirUrl.toString();

    m_currentStrategy = strategyName;
    saveSettings();
    
    emit groupingStrategyChanged(strategyName);
    
    fmDebug() << "Grouping strategy set successfully:" << strategyName;
}

QString GroupingManager::getGroupingStrategy() const
{
    return m_currentStrategy;
}

void GroupingManager::setGroupingEnabled(bool enabled)
{
    if (m_groupingEnabled == enabled) {
        fmDebug() << "Grouping enabled state unchanged, skipping for URL:" << m_dirUrl.toString();
        return;
    }

    fmInfo() << "Setting grouping enabled:" << enabled << "for URL:" << m_dirUrl.toString();
    
    m_groupingEnabled = enabled;
    saveSettings();
    
    emit groupingEnabledChanged(enabled);
    
    fmDebug() << "Grouping enabled state set successfully:" << enabled;
}

bool GroupingManager::isGroupingEnabled() const
{
    return m_groupingEnabled;
}

void GroupingManager::setGroupingOrder(Qt::SortOrder order)
{
    if (m_groupingOrder == order) {
        fmDebug() << "Grouping order unchanged, skipping for URL:" << m_dirUrl.toString();
        return;
    }

    fmInfo() << "Setting grouping order:" << (order == Qt::AscendingOrder ? "Ascending" : "Descending") 
             << "for URL:" << m_dirUrl.toString();
    
    m_groupingOrder = order;
    saveSettings();
    
    emit groupingOrderChanged(order);
    
    fmDebug() << "Grouping order set successfully:" << (order == Qt::AscendingOrder ? "Ascending" : "Descending");
}

Qt::SortOrder GroupingManager::getGroupingOrder() const
{
    return m_groupingOrder;
}

void GroupingManager::toggleGroupExpansion(const QString &groupKey)
{
    bool currentState = m_groupExpansionStates.value(groupKey, true);
    bool newState = !currentState;
    
    fmDebug() << "Toggling group expansion:" << groupKey << "from" << currentState << "to" << newState;
    
    m_groupExpansionStates[groupKey] = newState;
    
    // Save expansion state
    QString stateKey = QString("groupExpansion_%1").arg(groupKey);
    WorkspaceHelper::instance()->setFileViewStateValue(m_dirUrl, stateKey, newState);
    
    emit groupExpansionChanged(groupKey, newState);
    
    fmDebug() << "Group expansion toggled successfully for key:" << groupKey;
}

bool GroupingManager::isGroupExpanded(const QString &groupKey) const
{
    return m_groupExpansionStates.value(groupKey, true);
}

void GroupingManager::applyToWorker(QSharedPointer<FileSortWorker> worker)
{
    if (worker.isNull()) {
        fmWarning() << "Cannot apply grouping settings: worker is null";
        return;
    }

    fmDebug() << "Applying grouping settings to worker - strategy:" << m_currentStrategy 
              << "enabled:" << m_groupingEnabled 
              << "order:" << (m_groupingOrder == Qt::AscendingOrder ? "Ascending" : "Descending");

    try {
        // Create strategy instance
        auto strategy = createStrategy(m_currentStrategy);
        if (!strategy) {
            fmCritical() << "Failed to create grouping strategy:" << m_currentStrategy;
            return;
        }

        // Apply strategy, settings, and order to worker
        worker->setGroupingStrategy(std::move(strategy));
        worker->setGroupingEnabled(m_groupingEnabled);
        
        // Set grouping order in worker
        worker->setGroupOrder(m_groupingOrder);

        fmDebug() << "Successfully applied grouping settings to worker";

    } catch (const std::exception &e) {
        fmCritical() << "Exception while applying grouping settings to worker:" << e.what();
    } catch (...) {
        fmCritical() << "Unknown exception while applying grouping settings to worker";
    }
}

void GroupingManager::restoreSettings()
{
    fmDebug() << "Restoring grouping settings for URL:" << m_dirUrl.toString();

    // Restore strategy, enabled state, and order
    QString savedStrategy = WorkspaceHelper::instance()->getFileViewStateValue(m_dirUrl, "groupStrategy", "NoGroupStrategy").toString();
    bool savedEnabled = WorkspaceHelper::instance()->getFileViewStateValue(m_dirUrl, "groupingEnabled", false).toBool();
    Qt::SortOrder savedOrder = static_cast<Qt::SortOrder>(WorkspaceHelper::instance()->getFileViewStateValue(m_dirUrl, "groupingOrder", Qt::AscendingOrder).toInt());
    
    m_currentStrategy = savedStrategy;
    m_groupingEnabled = savedEnabled;
    m_groupingOrder = savedOrder;
    
    // Load expansion states
    loadExpansionStates();
    
    fmDebug() << "Restored grouping settings - strategy:" << m_currentStrategy 
              << "enabled:" << m_groupingEnabled 
              << "order:" << (m_groupingOrder == Qt::AscendingOrder ? "Ascending" : "Descending")
              << "for URL:" << m_dirUrl.toString();
}

QStringList GroupingManager::getSupportedStrategies()
{
    if (s_supportedStrategies.isEmpty()) {
        // Initialize supported strategies using actual strategy instances
        // This ensures we get the correct names from getStrategyName()
        
        auto noGroup = std::make_unique<NoGroupStrategy>();
        s_supportedStrategies << noGroup->getStrategyName();
        
        auto typeGroup = std::make_unique<TypeGroupStrategy>();
        s_supportedStrategies << typeGroup->getStrategyName();
        
        auto timeGroup = std::make_unique<TimeGroupStrategy>(TimeGroupStrategy::ModificationTime);
        s_supportedStrategies << timeGroup->getStrategyName();
        
        auto nameGroup = std::make_unique<NameGroupStrategy>();
        s_supportedStrategies << nameGroup->getStrategyName();
        
        auto sizeGroup = std::make_unique<SizeGroupStrategy>();
        s_supportedStrategies << sizeGroup->getStrategyName();
        
        fmDebug() << "Initialized supported strategies:" << s_supportedStrategies;
    }
    
    return s_supportedStrategies;
}

QString GroupingManager::roleToStrategyName(DFMGLOBAL_NAMESPACE::ItemRoles role)
{
    using namespace DFMGLOBAL_NAMESPACE;
    
    switch (role) {
    case Global::ItemRoles::kItemUnknowRole:
        return "NoGroupStrategy";
    case Global::ItemRoles::kItemFileMimeTypeRole:
        return "Type";
    case Global::ItemRoles::kItemFileLastModifiedRole:
        return "ModifiedTime";
    case Global::ItemRoles::kItemFileCreatedRole:
        // Note: TimeGroupStrategy can handle both modification and creation time
        // We'll use ModifiedTime as default and let the strategy determine behavior
        return "ModifiedTime"; 
    case Global::ItemRoles::kItemFileDisplayNameRole:
        return "Name";
    case Global::ItemRoles::kItemFileSizeRole:
        return "Size";
    default:
        fmWarning() << "Unknown ItemRole for grouping:" << static_cast<int>(role) << "defaulting to NoGroupStrategy";
        return "NoGroupStrategy";
    }
}

DFMGLOBAL_NAMESPACE::ItemRoles GroupingManager::strategyNameToRole(const QString &strategyName)
{
    using namespace DFMGLOBAL_NAMESPACE;
    
    if (strategyName == "NoGroupStrategy") {
        return Global::ItemRoles::kItemUnknowRole;
    } else if (strategyName == "Type") {
        return Global::ItemRoles::kItemFileMimeTypeRole;
    } else if (strategyName == "ModifiedTime") {
        return Global::ItemRoles::kItemFileLastModifiedRole;
    } else if (strategyName == "Name") {
        return Global::ItemRoles::kItemFileDisplayNameRole;
    } else if (strategyName == "Size") {
        return Global::ItemRoles::kItemFileSizeRole;
    } else {
        fmWarning() << "Unknown strategy name:" << strategyName << "defaulting to kItemUnknowRole";
        return Global::ItemRoles::kItemUnknowRole;
    }
}

std::unique_ptr<AbstractGroupStrategy> GroupingManager::createStrategy(const QString &strategyName) const
{
    fmDebug() << "Creating grouping strategy:" << strategyName;

    // Use strategy instances to get correct names, avoiding hardcoded strings
    auto noGroup = std::make_unique<NoGroupStrategy>();
    if (strategyName == noGroup->getStrategyName()) {
        return std::move(noGroup);
    }
    
    auto typeGroup = std::make_unique<TypeGroupStrategy>();
    if (strategyName == typeGroup->getStrategyName()) {
        return std::move(typeGroup);
    }
    
    auto timeGroup = std::make_unique<TimeGroupStrategy>(TimeGroupStrategy::ModificationTime);
    if (strategyName == timeGroup->getStrategyName()) {
        return std::move(timeGroup);
    }
    
    auto nameGroup = std::make_unique<NameGroupStrategy>();
    if (strategyName == nameGroup->getStrategyName()) {
        return std::move(nameGroup);
    }
    
    auto sizeGroup = std::make_unique<SizeGroupStrategy>();
    if (strategyName == sizeGroup->getStrategyName()) {
        return std::move(sizeGroup);
    }

    fmWarning() << "Unknown grouping strategy:" << strategyName << "defaulting to NoGroupStrategy";
    return std::make_unique<NoGroupStrategy>();
}

bool GroupingManager::validateStrategy(const QString &strategyName) const
{
    QStringList supportedStrategies = getSupportedStrategies();
    return supportedStrategies.contains(strategyName);
}

void GroupingManager::saveSettings()
{
    fmDebug() << "Saving grouping settings for URL:" << m_dirUrl.toString();
    
    WorkspaceHelper::instance()->setFileViewStateValue(m_dirUrl, "groupStrategy", m_currentStrategy);
    WorkspaceHelper::instance()->setFileViewStateValue(m_dirUrl, "groupingEnabled", m_groupingEnabled);
    WorkspaceHelper::instance()->setFileViewStateValue(m_dirUrl, "groupingOrder", static_cast<int>(m_groupingOrder));
    
    fmDebug() << "Grouping settings saved - strategy:" << m_currentStrategy 
              << "enabled:" << m_groupingEnabled 
              << "order:" << (m_groupingOrder == Qt::AscendingOrder ? "Ascending" : "Descending");
}

void GroupingManager::loadExpansionStates()
{
    // Group expansion states are loaded on-demand when groups are created
    // This is because we don't know which groups exist until grouping occurs
    fmDebug() << "Group expansion states will be loaded on-demand for URL:" << m_dirUrl.toString();
}

DPWORKSPACE_END_NAMESPACE 