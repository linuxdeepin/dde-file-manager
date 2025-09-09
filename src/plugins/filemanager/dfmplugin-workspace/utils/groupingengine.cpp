// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "groupingengine.h"

#include <dfm-base/dfm_log_defines.h>

#include <QMutexLocker>
#include <QElapsedTimer>
#include <QDebug>
#include <algorithm>

DPWORKSPACE_BEGIN_NAMESPACE

GroupingEngine::GroupingEngine(QObject *parent)
    : QObject(parent)
{
}

GroupingEngine::~GroupingEngine()
{
}

GroupingEngine::GroupingResult GroupingEngine::groupFiles(const QList<FileItemDataPointer> &files, 
                                                          AbstractGroupStrategy *strategy) const
{
    if (!strategy) {
        GroupingResult result;
        result.success = false;
        result.errorMessage = "Invalid grouping strategy";
        return result;
    }

    QElapsedTimer timer;
    timer.start();

    // Build cache key
    CacheKey currentKey{
        strategy->getStrategyName(),
        static_cast<int>(files.size()),
        m_groupOrder
    };

    // Check cache
    {
        QMutexLocker locker(&m_cacheMutex);
        if (m_cacheValid && currentKey == m_lastCacheKey) {
            QMutexLocker statsLocker(&m_statsMutex);
            ++m_cacheHits;
            fmDebug() << "GroupingEngine: Cache hit for strategy" 
                                          << strategy->getStrategyName() 
                                          << "with" << files.size() << "files";
            return m_cachedResult;
        }
    }

    // Cache miss - perform grouping
    {
        QMutexLocker statsLocker(&m_statsMutex);
        ++m_cacheMisses;
    }

    fmDebug() << "GroupingEngine: Performing grouping for strategy" 
                                  << strategy->getStrategyName() 
                                  << "with" << files.size() << "files";

    GroupingResult result = performGrouping(files, strategy);
    
    if (result.success) {
        // Sort groups by display order
        sortGroupsByDisplayOrder(result.groups, strategy);
        
        // Update cache
        QMutexLocker locker(&m_cacheMutex);
        m_lastCacheKey = currentKey;
        m_cachedResult = result;
        m_cacheValid = true;
    }

    fmDebug() << "GroupingEngine: Grouping completed in" 
                                  << timer.elapsed() << "ms, created" 
                                  << result.groups.size() << "groups";

    return result;
}

void GroupingEngine::sortGroups(QList<FileGroupData> &groups, 
                               AbstractGroupStrategy *strategy, 
                               Qt::SortOrder order) const
{
    if (!strategy || groups.isEmpty()) {
        return;
    }

    std::sort(groups.begin(), groups.end(), 
             [strategy, order](const FileGroupData &left, const FileGroupData &right) {
                 int leftOrder = strategy->getGroupDisplayOrder(left.groupKey, order);
                 int rightOrder = strategy->getGroupDisplayOrder(right.groupKey, order);
                 return leftOrder < rightOrder;
             });
}

GroupedModelData GroupingEngine::generateModelData(const GroupingResult &groupingResult,
                                                  const QHash<QString, bool> &expansionStates) const
{
    GroupedModelData modelData;
    
    if (!groupingResult.success) {
        fmWarning() << "GroupingEngine: Cannot generate model data from failed grouping result";
        return modelData;
    }

    // Copy groups and set expansion states
    modelData.groups = groupingResult.groups;
    modelData.groupExpansionStates = expansionStates;
    
    // Update group expansion states in the group data
    for (auto &group : modelData.groups) {
        group.isExpanded = expansionStates.value(group.groupKey, true);
    }

    // Build the flattened items list
    modelData.rebuildFlattenedItems();

    fmDebug() << "GroupingEngine: Generated model data with" 
                                  << modelData.groups.size() << "groups and" 
                                  << modelData.getItemCount() << "flattened items";

    return modelData;
}

void GroupingEngine::invalidateCache()
{
    QMutexLocker locker(&m_cacheMutex);
    m_cacheValid = false;
    fmDebug() << "GroupingEngine: Cache invalidated";
}

void GroupingEngine::setGroupOrder(Qt::SortOrder order)
{
    if (m_groupOrder != order) {
        m_groupOrder = order;
        invalidateCache(); // Order change invalidates cache
        fmDebug() << "GroupingEngine: Group order changed to" 
                                      << (order == Qt::AscendingOrder ? "Ascending" : "Descending");
    }
}

int GroupingEngine::getCacheHits() const
{
    QMutexLocker locker(&m_statsMutex);
    return m_cacheHits;
}

int GroupingEngine::getCacheMisses() const
{
    QMutexLocker locker(&m_statsMutex);
    return m_cacheMisses;
}

void GroupingEngine::resetCacheStats()
{
    QMutexLocker locker(&m_statsMutex);
    m_cacheHits = 0;
    m_cacheMisses = 0;
    fmDebug() << "GroupingEngine: Cache statistics reset";
}

GroupingEngine::GroupingResult GroupingEngine::performGrouping(const QList<FileItemDataPointer> &files,
                                                               AbstractGroupStrategy *strategy) const
{
    GroupingResult result;
    
    try {
        // Use QHash for efficient grouping (O(1) average insertion time)
        QHash<QString, QList<FileItemDataPointer>> groupMap;
        
        // Reserve space for better performance
        groupMap.reserve(qMin(files.size() / 4 + 1, 50)); // Reasonable estimate
        
        // Single pass grouping
        for (const auto &file : files) {
            if (!file) {
                continue; // Skip null pointers
            }
            
            // Convert FileItemDataPointer to FileInfoPointer for strategy interface
            FileInfoPointer fileInfo = file->fileInfo();
            if (!fileInfo) {
                fmWarning() << "GroupingEngine: Invalid file info for" << file->data(DFMBASE_NAMESPACE::Global::kItemUrlRole).toUrl();
                continue;
            }
            
            QString groupKey = strategy->getGroupKey(fileInfo);
            if (groupKey.isEmpty()) {
                fmWarning() << "GroupingEngine: Empty group key for file" << file->data(DFMBASE_NAMESPACE::Global::kItemUrlRole).toUrl();
                continue;
            }
            
            groupMap[groupKey].append(file);
        }
        
        // Convert hash map to result groups
        result.groups.reserve(groupMap.size());
        
        for (auto it = groupMap.constBegin(); it != groupMap.constEnd(); ++it) {
            const QString &groupKey = it.key();
            const QList<FileItemDataPointer> &groupFiles = it.value();
            
            // Convert FileItemDataPointer list to FileInfoPointer list for strategy interface
            QList<FileInfoPointer> groupFileInfos;
            groupFileInfos.reserve(groupFiles.size());
            for (const auto &file : groupFiles) {
                if (file && file->fileInfo()) {
                    groupFileInfos.append(file->fileInfo());
                }
            }
            
            // Skip groups that shouldn't be visible
            if (!strategy->isGroupVisible(groupKey, groupFileInfos)) {
                continue;
            }
            
            FileGroupData group;
            group.groupKey = groupKey;
            group.displayName = strategy->getGroupDisplayName(groupKey);
            group.files = groupFiles;
            group.fileCount = groupFiles.size();
            group.isExpanded = true; // Default to expanded
            group.displayOrder = strategy->getGroupDisplayOrder(groupKey, m_groupOrder);
            
            result.groups.append(group);
        }
        
        result.success = true;
        
    } catch (const std::exception &e) {
        result.success = false;
        result.errorMessage = QString("Grouping failed: %1").arg(e.what());
        fmCritical() << "GroupingEngine: Exception during grouping:" << e.what();
    } catch (...) {
        result.success = false;
        result.errorMessage = "Unknown error during grouping";
        fmCritical() << "GroupingEngine: Unknown exception during grouping";
    }
    
    return result;
}

void GroupingEngine::sortGroupsByDisplayOrder(QList<FileGroupData> &groups, 
                                             AbstractGroupStrategy *strategy) const
{
    if (!strategy || groups.isEmpty()) {
        return;
    }

    // Sort groups by their display order
    std::sort(groups.begin(), groups.end(),
             [strategy, this](const FileGroupData &left, const FileGroupData &right) {
                 int leftOrder = strategy->getGroupDisplayOrder(left.groupKey, m_groupOrder);
                 int rightOrder = strategy->getGroupDisplayOrder(right.groupKey, m_groupOrder);
                 return leftOrder < rightOrder;
             });
}

DPWORKSPACE_END_NAMESPACE 