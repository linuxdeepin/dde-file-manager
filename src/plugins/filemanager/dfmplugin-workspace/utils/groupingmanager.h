// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GROUPINGMANAGER_H
#define GROUPINGMANAGER_H

#include "dfmplugin_workspace_global.h"
#include "filesortworker.h"

#include <dfm-base/interfaces/abstractgroupstrategy.h>

#include <QObject>
#include <QString>
#include <QUrl>
#include <QHash>
#include <memory>

DPWORKSPACE_BEGIN_NAMESPACE

class WorkspaceHelper;

/**
 * @brief Manages all grouping-related operations for FileViewModel
 * 
 * This class encapsulates grouping strategy management, state persistence,
 * and interaction with FileSortWorker, following the Single Responsibility Principle.
 */
class GroupingManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param dirUrl The directory URL this manager is responsible for
     * @param parent Parent QObject
     */
    explicit GroupingManager(const QUrl &dirUrl, QObject *parent = nullptr);

    /**
     * @brief Destructor
     */
    ~GroupingManager();

    /**
     * @brief Set the grouping strategy
     * @param strategyName The strategy name (should match strategy's getStrategyName())
     */
    void setGroupingStrategy(const QString &strategyName);

    /**
     * @brief Get the current grouping strategy name
     * @return Current strategy name
     */
    QString getGroupingStrategy() const;

    /**
     * @brief Enable or disable grouping
     * @param enabled True to enable, false to disable
     */
    void setGroupingEnabled(bool enabled);

    /**
     * @brief Check if grouping is enabled
     * @return True if enabled, false otherwise
     */
    bool isGroupingEnabled() const;

    /**
     * @brief Set the grouping order (ascending/descending)
     * @param order The sort order for groups
     */
    void setGroupingOrder(Qt::SortOrder order);

    /**
     * @brief Get the current grouping order
     * @return Current grouping order
     */
    Qt::SortOrder getGroupingOrder() const;

    /**
     * @brief Toggle group expansion state
     * @param groupKey The group key to toggle
     */
    void toggleGroupExpansion(const QString &groupKey);

    /**
     * @brief Check if a group is expanded
     * @param groupKey The group key to check
     * @return True if expanded, false otherwise
     */
    bool isGroupExpanded(const QString &groupKey) const;

    /**
     * @brief Apply current grouping settings to a FileSortWorker
     * @param worker The worker to apply settings to
     */
    void applyToWorker(QSharedPointer<FileSortWorker> worker);

    /**
     * @brief Restore grouping settings from persistent storage
     */
    void restoreSettings();

    /**
     * @brief Get list of supported strategy names
     * @return List of strategy names that can be used with setGroupingStrategy
     */
    static QStringList getSupportedStrategies();

    /**
     * @brief Convert ItemRoles to corresponding strategy name
     * @param role The ItemRoles to convert
     * @return Strategy name, or "NoGroupStrategy" for unknown roles
     */
    static QString roleToStrategyName(DFMGLOBAL_NAMESPACE::ItemRoles role);

    /**
     * @brief Convert strategy name to corresponding ItemRoles
     * @param strategyName The strategy name to convert
     * @return ItemRoles, or kItemUnknowRole for unknown strategies
     */
    static DFMGLOBAL_NAMESPACE::ItemRoles strategyNameToRole(const QString &strategyName);

Q_SIGNALS:
    /**
     * @brief Emitted when grouping strategy changes
     * @param strategyName The new strategy name
     */
    void groupingStrategyChanged(const QString &strategyName);

    /**
     * @brief Emitted when grouping enabled state changes
     * @param enabled The new enabled state
     */
    void groupingEnabledChanged(bool enabled);

    /**
     * @brief Emitted when grouping order changes
     * @param order The new grouping order
     */
    void groupingOrderChanged(Qt::SortOrder order);

    /**
     * @brief Emitted when group expansion state changes
     * @param groupKey The group key
     * @param expanded The new expansion state
     */
    void groupExpansionChanged(const QString &groupKey, bool expanded);

private:
    /**
     * @brief Create a grouping strategy instance by name
     * @param strategyName The strategy name
     * @return Unique pointer to the strategy, or nullptr if unknown
     */
    std::unique_ptr<DFMBASE_NAMESPACE::AbstractGroupStrategy> createStrategy(const QString &strategyName) const;

    /**
     * @brief Validate if a strategy name is supported
     * @param strategyName The strategy name to validate
     * @return True if valid, false otherwise
     */
    bool validateStrategy(const QString &strategyName) const;

    /**
     * @brief Save current settings to persistent storage
     */
    void saveSettings();

    /**
     * @brief Load group expansion states from persistent storage
     */
    void loadExpansionStates();

private:
    QUrl m_dirUrl;                              ///< Directory URL this manager handles
    QString m_currentStrategy;                  ///< Current grouping strategy name
    bool m_groupingEnabled;                     ///< Whether grouping is enabled
    Qt::SortOrder m_groupingOrder;             ///< Grouping order (ascending/descending)
    QHash<QString, bool> m_groupExpansionStates; ///< Group expansion states

    static QStringList s_supportedStrategies;  ///< Cache of supported strategy names
};

DPWORKSPACE_END_NAMESPACE

#endif // GROUPINGMANAGER_H 