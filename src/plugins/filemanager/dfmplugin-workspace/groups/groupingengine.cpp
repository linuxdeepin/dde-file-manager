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

GroupingEngine::UpdateResult GroupingEngine::removeFilesFromModelData(const GroupedModelData &oldData)
{
    GroupingEngine::UpdateResult result;

    if (m_updateMode != UpdateMode::kRemove || m_visibleChildrenForUpdate.isEmpty()) {
        fmWarning() << "GroupingEngine: Cannot remove files from model data without a valid update mode";
        return result;
    }

    result.newData = oldData;
    result.pos = result.newData.findFileStartPos(m_visibleChildrenForUpdate.first()).value_or(-1);
    result.count = m_visibleChildrenForUpdate.count();

    bool success = true;
    QSet<QString> updatedGroups;   // Track which groups need to be updated
    bool groupRemoved = false;
    for (auto &url : std::as_const(m_visibleChildrenForUpdate)) {
        int pos = result.newData.findFileStartPos(url).value_or(-1);
        if (pos < 0) {
            success = false;
            fmWarning() << "GroupingEngine: File" << url << "not found in model data";
            break;
        }

        const auto &wrapper = result.newData.getItemAt(pos);
        if (!wrapper.isFileItem() || wrapper.fileData.isNull()) {
            success = false;
            fmWarning() << "GroupingEngine: File" << url << "is not a file item";
            break;
        }

        FileGroupData *groupData = result.newData.getGroup(wrapper.groupKey);
        if (!groupData) {
            success = false;
            fmWarning() << "GroupingEngine: Group" << groupData->groupKey << "not found in model data";
            break;
        }

        if (!groupData->removeFile(url)) {
            success = false;
            fmWarning() << "GroupingEngine: File" << url << "not removed from group" << groupData->groupKey;
            break;
        }

        // Track this group for update
        updatedGroups.insert(groupData->groupKey);

        // Remove group from model data if it is empty
        if (groupData->isEmpty()) {
            result.newData.removeGroup(groupData->groupKey);
            groupRemoved = true;
        }
    }

    result.success = success;
    if (result.success) {
        if (groupRemoved) {
            fmInfo() << "GroupingEngine: Due to an empty group, rebuilding flattened items";
            result.pos = 0;
            result.count = result.newData.getItemCount();
            result.newData.rebuildFlattenedItems();
        } else {
            fmDebug() << "GroupingEngine: Removing" << result.count << "items from model data at position" << result.pos;
            result.newData.removeItems(result.pos, result.count);

            // Update only the group headers that were affected
            for (const QString &groupKey : std::as_const(updatedGroups)) {
                result.newData.updateGroupHeader(groupKey);
            }
        }
    }

    return result;
}

QPair<int, int> GroupingEngine::currentUpdateChildrenRange() const
{
    return m_visibleChildrenRangeForUpdate;
}

GroupingEngine::UpdateMode GroupingEngine::currentUpdateMode() const
{
    return m_updateMode;
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
        sortGroupsByDisplayOrder(result.groups);
    }

    fmDebug() << "GroupingEngine: Grouping completed in"
              << timer.elapsed() << "ms, created"
              << result.groups.size() << "groups";

    return result;
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

void GroupingEngine::setVisibleTreeChildren(QHash<QUrl, QList<QUrl>> *children)
{
    m_visibleTreeChildren = children;
}

void GroupingEngine::setChildrenDataMap(QHash<QUrl, FileItemDataPointer> *map)
{
    m_childrenDataMap = map;
}

void GroupingEngine::setUpdateMode(UpdateMode mode)
{
    m_updateMode = mode;
}

void GroupingEngine::setUpdateChildren(const QList<QUrl> &children)
{
    m_visibleChildrenForUpdate = children;
}

void GroupingEngine::setUpdateChildrenRange(int pos, int count)
{
    m_visibleChildrenRangeForUpdate = qMakePair(pos, count);
}

void GroupingEngine::reorderGroups(GroupedModelData *modelData) const
{
    if (!modelData || modelData->groups.isEmpty()) {
        return;
    }

    fmDebug() << "GroupingEngine: Reordering" << modelData->groups.size()
              << "groups with current order" << (m_groupOrder == Qt::AscendingOrder ? "Ascending" : "Descending");

    // Sort groups based on their displayOrder according to m_groupOrder
    sortGroupsByDisplayOrder(modelData->groups);
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
            // a item is expanded tree item
            const auto &expandedFiles = findExpandedFiles(file);
            if (!expandedFiles.isEmpty()) {
                groupMap[groupKey].append(expandedFiles);
            }
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
            group.displayOrder = strategy->getGroupDisplayOrder(groupKey);

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

void GroupingEngine::sortGroupsByDisplayOrder(QList<FileGroupData> &groups) const
{
    if (groups.isEmpty()) {
        return;
    }

    // Sort groups by their display order
    std::sort(groups.begin(), groups.end(),
              [this](const FileGroupData &left, const FileGroupData &right) {
                  if (m_groupOrder == Qt::AscendingOrder) {
                      return left.displayOrder < right.displayOrder;
                  } else {
                      return left.displayOrder > right.displayOrder;
                  }
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
        || strategyName == GroupStrategty::kCreatedTime
        || strategyName == GroupStrategty::kCustomPath
        || strategyName == GroupStrategty::kCustomTime) {
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

QList<FileItemDataPointer> GroupingEngine::findExpandedFiles(const FileItemDataPointer &file) const
{
    // 1. --- 前置安全检查 ---
    bool isExpanded = file->data(ItemRoles::kItemTreeViewExpandedRole).toBool();
    if (!isExpanded || !m_visibleTreeChildren || !m_childrenDataMap || file.isNull()) {
        return {};   // 如果初始节点未展开或数据结构无效，则其下没有任何“展开的文件”
    }

    const QUrl &fileUrl = file->data(ItemRoles::kItemUrlRole).toUrl();
    if (!fileUrl.isValid()) {
        return {};
    }

    // 2. --- 初始化 ---
    QList<FileItemDataPointer> expandedFiles;   // 最终返回的线性列表
    QStack<QUrl> urlsToProcess;   // 使用栈来模拟递归的深度优先（DFS）搜索

    // 3. --- 遍历 ---
    // 遍历从`file`的直接子项开始，将它们压入栈中
    auto initialChildrenIt = m_visibleTreeChildren->constFind(fileUrl);
    if (initialChildrenIt != m_visibleTreeChildren->constEnd()) {
        const QList<QUrl> &directChildren = initialChildrenIt.value();
        // 使用反向迭代器将子项逆序压入栈中，以保证处理时是正序
        for (auto it = directChildren.crbegin(); it != directChildren.crend(); ++it) {
            urlsToProcess.push(*it);
        }
    }

    // 4. --- 迭代遍历 ---
    // 当栈不为空时，说明还有节点需要处理
    while (!urlsToProcess.isEmpty()) {
        const QUrl currentUrl = urlsToProcess.pop();

        // 从数据映射中查找当前URL对应的FileItemDataPointer
        auto dataIt = m_childrenDataMap->constFind(currentUrl);
        if (dataIt == m_childrenDataMap->constEnd()) {
            // 数据不一致：URL在树中但不在数据映射中。
            fmWarning() << "Inconsistent data: URL not found in childrenDataMap:" << currentUrl;
            continue;   // 跳过这个无效的条目
        }

        const FileItemDataPointer &currentItem = dataIt.value();

        // 将当前项添加到结果列表中
        expandedFiles.append(currentItem);

        // 检查当前项是否也处于展开状态
        bool currentItemIsExpanded = currentItem->data(ItemRoles::kItemTreeViewExpandedRole).toBool();
        if (currentItemIsExpanded) {
            // 如果是展开的，将其子项压入栈中，以便在后续迭代中处理
            auto childrenIt = m_visibleTreeChildren->constFind(currentUrl);
            if (childrenIt != m_visibleTreeChildren->constEnd()) {
                const QList<QUrl> &childrenOfCurrent = childrenIt.value();
                // 同样，将其子项逆序压入栈中
                for (auto it = childrenOfCurrent.crbegin(); it != childrenOfCurrent.crend(); ++it) {
                    urlsToProcess.push(*it);
                }
            }
        }
    }

    return expandedFiles;
}

DPWORKSPACE_END_NAMESPACE
