// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GROUPINGENGINE_H
#define GROUPINGENGINE_H

#include "dfmplugin_workspace_global.h"
#include "filegroupdata.h"
#include "groupedmodeldata.h"
#include "groups/abstractgroupstrategy.h"

#include <QObject>
#include <QHash>
#include <QString>
#include <QList>
#include <QMutex>

DPWORKSPACE_BEGIN_NAMESPACE

/**
 * @brief Core engine for file grouping operations
 * 
 * This class implements the main grouping algorithms and manages
 * the transformation of file lists into grouped model data.
 */
class GroupingEngine : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Result structure for grouping operations
     */
    struct GroupingResult {
        QList<FileGroupData> groups;    ///< The grouped file data
        bool success = false;           ///< Whether the operation succeeded
        QString errorMessage;           ///< Error message if operation failed
    };

    /**
     * @brief Cache key for grouping results
     */
    struct CacheKey {
        QString strategyName;           ///< Strategy name
        int fileCount;                 ///< Number of files
        Qt::SortOrder groupOrder;      ///< Group sort order
        
        bool operator==(const CacheKey &other) const {
            return strategyName == other.strategyName 
                && fileCount == other.fileCount 
                && groupOrder == other.groupOrder;
        }
    };

    /**
     * @brief Constructor
     * @param parent Parent object
     */
    explicit GroupingEngine(QObject *parent = nullptr);

    /**
     * @brief Destructor
     */
    ~GroupingEngine();

    /**
     * @brief Group files using the specified strategy
     * @param files List of files to group
     * @param strategy The grouping strategy to use
     * @return The grouping result
     */
    GroupingResult groupFiles(const QList<FileItemDataPointer> &files, 
                             AbstractGroupStrategy *strategy) const;

    /**
     * @brief Sort groups according to strategy and order
     * @param groups List of groups to sort (modified in place)
     * @param strategy The grouping strategy
     * @param order Sort order for groups
     */
    void sortGroups(QList<FileGroupData> &groups, 
                   AbstractGroupStrategy *strategy, 
                   Qt::SortOrder order) const;

    /**
     * @brief Generate flattened model data from grouping result
     * @param groupingResult The result from groupFiles()
     * @param expansionStates Current expansion states for groups
     * @return The flattened model data ready for use in views
     */
    GroupedModelData generateModelData(const GroupingResult &groupingResult,
                                      const QHash<QString, bool> &expansionStates) const;

    /**
     * @brief Invalidate all cached results
     */
    void invalidateCache();

    /**
     * @brief Set the current group order
     * @param order The sort order for groups
     */
    void setGroupOrder(Qt::SortOrder order);

    /**
     * @brief Get cache hit statistics
     * @return Number of cache hits since last reset
     */
    int getCacheHits() const;

    /**
     * @brief Get cache miss statistics  
     * @return Number of cache misses since last reset
     */
    int getCacheMisses() const;

    /**
     * @brief Reset cache statistics
     */
    void resetCacheStats();

private:
    /**
     * @brief Perform the actual grouping algorithm
     * @param files List of files to group
     * @param strategy The grouping strategy
     * @return The grouping result
     */
    GroupingResult performGrouping(const QList<FileItemDataPointer> &files,
                                  AbstractGroupStrategy *strategy) const;

    /**
     * @brief Sort groups by their display order
     * @param groups List of groups to sort
     * @param strategy The grouping strategy
     */
    void sortGroupsByDisplayOrder(QList<FileGroupData> &groups, 
                                 AbstractGroupStrategy *strategy) const;

    // Cache management
    mutable CacheKey m_lastCacheKey;
    mutable GroupingResult m_cachedResult;
    mutable bool m_cacheValid = false;
    mutable QMutex m_cacheMutex;

    // Configuration
    Qt::SortOrder m_groupOrder = Qt::AscendingOrder;

    // Statistics
    mutable int m_cacheHits = 0;
    mutable int m_cacheMisses = 0;
    mutable QMutex m_statsMutex;
};

DPWORKSPACE_END_NAMESPACE

#endif // GROUPINGENGINE_H 
