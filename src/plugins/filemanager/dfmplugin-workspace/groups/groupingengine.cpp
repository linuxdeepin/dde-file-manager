// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "groupingengine.h"

#include <dfm-base/dfm_log_defines.h>
#include <dfm-base/base/schemefactory.h>

#include <QElapsedTimer>
#include <QDebug>
#include <algorithm>

DPWORKSPACE_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

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

    fmDebug() << "GroupingEngine: Performing grouping for strategy"
              << strategy->getStrategyName()
              << "with" << files.size() << "files";

    GroupingResult result = performGrouping(files, strategy);

    if (result.success) {
        // Sort groups by display order
        sortGroupsByDisplayOrder(result.groups, strategy);
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

void GroupingEngine::setGroupOrder(Qt::SortOrder order)
{
    if (m_groupOrder != order) {
        m_groupOrder = order;
        fmDebug() << "GroupingEngine: Group order changed to"
                  << (order == Qt::AscendingOrder ? "Ascending" : "Descending");
    }
}

void GroupingEngine::reorderGroups(GroupedModelData *modelData) const
{
    if (!modelData || modelData->groups.isEmpty()) {
        return;
    }

    fmDebug() << "GroupingEngine: Reordering" << modelData->groups.size()
              << "groups with current order" << (m_groupOrder == Qt::AscendingOrder ? "Ascending" : "Descending");

    // Sort groups based on their displayOrder according to m_groupOrder
    std::sort(modelData->groups.begin(), modelData->groups.end(),
              [this](const FileGroupData &left, const FileGroupData &right) {
                  if (m_groupOrder == Qt::AscendingOrder) {
                      return left.displayOrder < right.displayOrder;
                  } else {
                      return left.displayOrder > right.displayOrder;
                  }
              });

    modelData->rebuildFlattenedItems();
}

GroupingEngine::GroupingResult GroupingEngine::performGrouping(const QList<FileItemDataPointer> &files,
                                                               AbstractGroupStrategy *strategy) const
{
    GroupingResult result;

    try {
        // Use QHash for efficient grouping (O(1) average insertion time)
        QHash<QString, QList<FileItemDataPointer>> groupMap;

        // Reserve space for better performance
        groupMap.reserve(qMin(files.size() / 4 + 1, 50));   // Reasonable estimate

        // Single pass grouping
        for (const auto &file : files) {
            if (!file) {
                continue;   // Skip null pointers
            }

            // Convert FileItemDataPointer to FileInfoPointer for strategy interface
            FileInfoPointer fileInfo = file->fileInfo();
            if (!fileInfo) {
                fileInfo = InfoFactory::create<FileInfo>(file->data(ItemRoles::kItemUrlRole).toUrl());
                Q_ASSERT(fileInfo);
                if (!fileInfo) {
                    fmWarning() << "GroupingEngine: Invalid file info for" << file->data(DFMBASE_NAMESPACE::Global::kItemUrlRole).toUrl();
                    continue;
                }
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

            // Early optimization: Skip empty groups for performance
            // Most strategies (except NoGroupStrategy) simply check !infos.isEmpty()
            if (groupFiles.isEmpty()) {
                continue;
            }

            // Create group data first
            FileGroupData group;
            group.groupKey = groupKey;
            group.displayName = strategy->getGroupDisplayName(groupKey);
            group.files = groupFiles;
            group.fileCount = groupFiles.size();
            group.isExpanded = true;   // Default to expanded
            group.displayOrder = strategy->getGroupDisplayOrder(groupKey, m_groupOrder);

            // Check group visibility with converted file infos if needed
            if (!isGroupVisibleWithConversion(groupKey, groupFiles, strategy)) {
                continue;
            }

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

bool GroupingEngine::isGroupVisibleWithConversion(const QString &groupKey,
                                                  const QList<FileItemDataPointer> &groupFiles,
                                                  AbstractGroupStrategy *strategy) const
{
    if (!strategy) {
        return false;
    }

    // Performance optimization: For built-in workspace strategies that simply check !infos.isEmpty(),
    // we can avoid the expensive file info conversion and just check if groupFiles is non-empty
    const QString strategyName = strategy->getStrategyName();

    // Check if this is one of the built-in strategies that only check for non-empty lists
    // These strategies' isGroupVisible simply returns !infos.isEmpty()
    if (strategyName == GroupStrategty::kName
        || strategyName == GroupStrategty::kSize
        || strategyName == GroupStrategty::kType
        || strategyName == GroupStrategty::kModifiedTime
        || strategyName == GroupStrategty::kCreatedTime) {
        fmDebug() << "GroupingEngine: Fast path for built-in strategy" << strategyName
                  << "- skipping file info conversion";
        return !groupFiles.isEmpty();
    }

    // For NoGroupStrategy, it always returns false (groups are not visible in no-group mode)
    if (strategyName == GroupStrategty::kNoGroup) {
        fmDebug() << "GroupingEngine: NoGroupStrategy always returns false for group visibility";
        return false;
    }

    // For unknown/custom strategies, perform the full conversion and call isGroupVisible
    fmDebug() << "GroupingEngine: Using full conversion path for custom strategy" << strategyName;
    QList<FileInfoPointer> groupFileInfos;
    groupFileInfos.reserve(groupFiles.size());

    for (const auto &file : groupFiles) {
        if (file && file->fileInfo()) {
            groupFileInfos.append(file->fileInfo());
        }
    }

    return strategy->isGroupVisible(groupKey, groupFileInfos);
}

DPWORKSPACE_END_NAMESPACE
