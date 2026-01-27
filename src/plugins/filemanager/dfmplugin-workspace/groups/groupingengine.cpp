// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "groupingengine.h"

#include <dfm-base/dfm_log_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/protocolutils.h>

#include <QElapsedTimer>
#include <QDebug>
#include <algorithm>

DPWORKSPACE_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

GroupingEngine::GroupingEngine(const QUrl &rootUrl, QObject *parent)
    : QObject(parent), m_rootUrl(rootUrl)
{
}

GroupingEngine::~GroupingEngine()
{
}

GroupingEngine::UpdateResult GroupingEngine::updateFilesToModelData(const QUrl &anchorUrl, const GroupedModelData &oldData, dfmbase::AbstractGroupStrategy *strategy)
{
    GroupingEngine::UpdateResult result;

    if (m_updateMode != UpdateMode::kUpdate || m_visibleChildrenForUpdate.isEmpty() || !strategy || !anchorUrl.isValid()) {
        fmWarning() << "GroupingEngine: Cannot update files to model data without a valid update mode";
        return result;
    }

    if (!m_childrenDataMap) {
        fmWarning() << "GroupingEngine: Cannot update files without a valid children data map";
        return result;
    }

    result.newData = oldData;
    result.success = true;
    result.alwaysUpdate = false;

    // Collect FileItemDataPointer for all files to be updated
    QList<FileItemDataPointer> filesToUpdate;
    if (!collectFilesToInsert(&filesToUpdate)) {   // Reuse existing method to collect files
        result.success = false;
        return result;
    }

    if (filesToUpdate.size() != m_visibleChildrenForUpdate.size()) {
        fmWarning() << "GroupingEngine: Number of files to update does not match number of visible children";
        result.success = false;
        return result;
    }

    // Get group key for the files
    const QString groupKey = getGroupKeyForFiles(filesToUpdate, anchorUrl, strategy);
    if (groupKey.isEmpty()) {
        result.success = false;
        return result;
    }

    if (!processFilesAndUpdateGroups(filesToUpdate, groupKey, anchorUrl, &result.newData)) {
        result.success = false;
        fmWarning() << "GroupingEngine: Failed to update files to model data";
        return result;
    }

    reorderGroups(&result.newData);
    // TODO: perf
    result.pos = 0;
    result.count = result.newData.getItemCount();

    return result;
}

GroupingEngine::UpdateResult GroupingEngine::insertFilesToModelData(const QUrl &anchorUrl,
                                                                    const GroupedModelData &oldData,
                                                                    DFMBASE_NAMESPACE::AbstractGroupStrategy *strategy)
{
    GroupingEngine::UpdateResult result;

    if (m_updateMode != UpdateMode::kInsert || m_visibleChildrenForUpdate.isEmpty() || !strategy) {
        fmWarning() << "GroupingEngine: Cannot insert files to model data without a valid update mode";
        return result;
    }

    if (!m_childrenDataMap) {
        fmWarning() << "GroupingEngine: Cannot insert files without a valid children data map";
        return result;
    }

    result.newData = oldData;
    result.success = true;
    result.alwaysUpdate = false;

    // Collect FileItemDataPointer for all files to be inserted
    QList<FileItemDataPointer> filesToInsert;
    if (!collectFilesToInsert(&filesToInsert)) {
        result.success = false;
        return result;
    }

    if (filesToInsert.size() != m_visibleChildrenForUpdate.size()) {
        fmWarning() << "GroupingEngine: Number of files to insert does not match number of visible children";
        result.success = false;
        return result;
    }

    // Get group key for the files
    QString groupKey = getGroupKeyForFiles(filesToInsert, anchorUrl, strategy);
    if (groupKey.isEmpty()) {
        result.success = false;
        return result;
    }

    // Process each file to insert and update groups
    bool alwaysUpdate = false;
    if (!processFilesAndInsertGroups(filesToInsert, groupKey, strategy,
                                     anchorUrl, &result.newData, &alwaysUpdate)) {
        result.success = false;
        result.alwaysUpdate = alwaysUpdate;
        fmWarning() << "GroupingEngine: Failed to insert files to model data";
        return result;
    }

    reorderGroups(&result.newData);
    // TODO: perf
    result.pos = 0;
    result.count = result.newData.getItemCount();
    return result;
}

QString GroupingEngine::getGroupKeyForFiles(const QList<FileItemDataPointer> &filesToInsert,
                                            const QUrl &anchorUrl,
                                            DFMBASE_NAMESPACE::AbstractGroupStrategy *strategy) const
{
    QString groupKey;
    FileItemDataPointer groupKeyItem;

    // 区分新增的文件是树形item展开的，还是当前目录新增的
    // 如果是树形展开的文件，其所属分组应该属于anchorUrl的分组
    auto anchorPointer = m_childrenDataMap->value(anchorUrl);
    int anchorExpandedCount = !anchorPointer.isNull() ? findExpandedFiles(anchorPointer).size() : 0;
    if (anchorExpandedCount == 0) {
        // 文件新增
        groupKeyItem = filesToInsert.first();
    } else {
        // 树形item展开
        const QUrl &topLevelUrl = findTopLevelAncestorOf(anchorUrl);
        groupKeyItem = m_childrenDataMap->value(topLevelUrl);
    }

    if (!groupKeyItem.isNull()) {
        auto groupKeyInfo = getFileInfoFromFileItem(groupKeyItem);
        groupKey = strategy->getGroupKey(groupKeyInfo);
    }

    return groupKey;
}

bool GroupingEngine::collectFilesToInsert(QList<FileItemDataPointer> *filesToInsert) const
{
    filesToInsert->reserve(m_visibleChildrenForUpdate.size());
    for (const QUrl &url : std::as_const(m_visibleChildrenForUpdate)) {
        // Check for cancellation during collection
        if (shouldCancel()) {
            fmInfo() << "GroupingEngine: File collection canceled by user";
            return false;
        }

        auto it = m_childrenDataMap->constFind(url);
        if (it == m_childrenDataMap->constEnd()) {
            fmWarning() << "GroupingEngine: File data not found for URL" << url;
            return false;
        }
        filesToInsert->append(it.value());
    }
    return true;
}

bool GroupingEngine::processFilesAndInsertGroups(const QList<FileItemDataPointer> &filesToInsert,
                                                 const QString groupKey,
                                                 const AbstractGroupStrategy *strategy,
                                                 const QUrl &anchorUrl,
                                                 GroupedModelData *newData,
                                                 bool *alwaysUpdate) const
{
    int index = -1;
    // Get or create the group
    FileGroupData *groupData = newData->getGroup(groupKey);

    // Process each file to insert
    for (const FileItemDataPointer &file : std::as_const(filesToInsert)) {
        // Check for cancellation during file processing
        if (shouldCancel()) {
            fmInfo() << "GroupingEngine: File insertion processing canceled by user";
            return false;
        }

        if (!file) {
            fmWarning() << "GroupingEngine: Invalid file data";
            continue;
        }

        if (groupKey.isEmpty()) {
            fmWarning() << "GroupingEngine: Empty group key for file" << file->data(DFMBASE_NAMESPACE::Global::kItemUrlRole).toUrl();
            continue;
        }

        if (!groupData) {
            // Create a new group
            FileGroupData newGroup;
            newGroup.groupKey = groupKey;
            newGroup.displayName = strategy->getGroupDisplayName(groupKey);
            newGroup.isExpanded = true;
            newGroup.displayOrder = strategy->getGroupDisplayOrder(groupKey);
            if (!newData->addGroup(newGroup)) {
                fmWarning() << "GroupingEngine: Failed to add group" << groupKey;
                return false;
            }
            groupData = newData->getGroup(groupKey);
            if (!groupData) {
                fmWarning() << "GroupingEngine: Failed to create or retrieve group" << groupKey;
                return false;
            }
            groupData->addFile(file);
        } else {
            if (index == -1) {
                index = groupData->findFileIndex(anchorUrl).value_or(-1);
                // 由于分组导致visiblechilren不连续，anchorUrl可能不在组中。
                if (index < 0) {
                    index = findNewAnchorPos(anchorUrl, groupData).value_or(0);
                    fmInfo() << "GroupingEngine: anchorUrl not found in group, index set to " << index;
                } else {
                    // 数据插入到 anchorUrl 之后
                    index += 1;
                }
            }
            Q_ASSERT(index >= 0);
            groupData->insertFile(index++, file);
        }

        if (!groupData->isExpanded) {
            fmWarning() << "GroupingEngine: group is collapsed:" << groupKey;
            *alwaysUpdate = true;
            // Update group headers before returning
            newData->updateGroupHeader(groupKey);
            return false;
        }
    }
    return true;
}

bool GroupingEngine::processFilesAndUpdateGroups(const QList<FileItemDataPointer> &filesToInsert,
                                                 const QString groupKey, const QUrl &anchorUrl,
                                                 GroupedModelData *newData) const
{
    if (groupKey.isEmpty() || !newData) {
        fmWarning() << "GroupingEngine: Empty group key ";
        return false;
    }

    FileGroupData *groupData = newData->getGroup(groupKey);
    if (!groupData) {
        return false;
    }

    int index = -1;
    // Process each file to update
    for (const FileItemDataPointer &file : std::as_const(filesToInsert)) {
        // Check for cancellation during file update processing
        if (shouldCancel()) {
            fmInfo() << "GroupingEngine: File update processing canceled by user";
            return false;
        }

        if (!file) {
            fmWarning() << "GroupingEngine: Invalid file data";
            return false;
        }

        if (index == -1) {
            index = groupData->findFileIndex(anchorUrl).value_or(-1);
            // 由于分组导致visiblechilren不连续，anchorUrl可能不在组中。
            if (index < 0) {
                index = findNewAnchorPos(anchorUrl, groupData).value_or(0);
                fmInfo() << "GroupingEngine: anchorUrl not found in group, index set to " << index;
            } else {
                // 数据插入到 anchorUrl 之后
                index += 1;
            }
        }
        Q_ASSERT(index >= 0);
        groupData->replaceFile(index++, file);
    }

    return true;
}

std::optional<int> GroupingEngine::findNewAnchorPos(const QUrl &oldAnchorUrl, const FileGroupData *group) const
{
    Q_ASSERT(m_visibleChildren);
    Q_ASSERT(group);
    QUrl newAnchorUrl;
    int index = m_visibleChildren->indexOf(oldAnchorUrl);
    if (index <= 0) {
        fmWarning() << "GroupingEngine: Old anchor URL not found in group";
        return std::nullopt;
    }

    index -= 1;   // ignore oldAnchorUrl index
    for (int i = index; i >= 0; --i) {
        const auto &url = m_visibleChildren->at(i);
        const auto &fileIndex = group->findFileIndex(url);
        if (fileIndex.has_value()) {
            return fileIndex.value() + 1;
        }
    }

    return std::nullopt;
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
        // Check for cancellation during file removal
        if (shouldCancel()) {
            fmInfo() << "GroupingEngine: File removal canceled by user";
            result.success = false;
            return result;
        }

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

std::optional<QUrl> GroupingEngine::findPrecedingAnchor(const QList<QUrl> &container, const QPair<int, int> &sliceRange)
{
    const int sliceStartIndex = sliceRange.first;

    // 安全检查：范围是否有效
    if (sliceStartIndex < 0 || sliceStartIndex + sliceRange.second > container.size()) {
        // 范围无效，无法确定锚点
        return std::nullopt;
    }

    // 如果切片的起始索引大于0，说明它前面有元素，可以作为锚点
    if (sliceStartIndex > 0) {
        return container.at(sliceStartIndex - 1);
    }

    // 切片位于容器的开头，没有前置锚点
    return QUrl();
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

void GroupingEngine::setVisibleChildren(QList<QUrl> *visibleChildren)
{
    m_visibleChildren = visibleChildren;
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

void GroupingEngine::setCancellationCheckCallback(CancellationCheckCallback callback)
{
    m_cancellationCheck = callback;
}

void GroupingEngine::reorderGroups(GroupedModelData *modelData) const
{
    if (!modelData || modelData->groups.isEmpty()) {
        fmWarning() << "GroupingEngine: Cannot reorder groups from empty model data";
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
            // Check for cancellation at the beginning of each iteration
            if (shouldCancel()) {
                fmInfo() << "GroupingEngine: Grouping operation canceled by user";
                result.success = false;
                result.errorMessage = "Operation canceled";
                return result;
            }

            if (!file) {
                continue;   // Skip null pointers
            }

            // Convert FileItemDataPointer to FileInfoPointer for strategy interface
            FileInfoPointer fileInfo = file->fileInfo();
            if (!fileInfo) {
                fileInfo = getFileInfoFromFileItem(file);
                if (!fileInfo) {
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
            // Check for cancellation when processing groups
            if (shouldCancel()) {
                fmInfo() << "GroupingEngine: Group conversion canceled by user";
                result.success = false;
                result.errorMessage = "Operation canceled";
                return result;
            }

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
    if (strategyName == GroupStrategy::kName
        || strategyName == GroupStrategy::kSize
        || strategyName == GroupStrategy::kType
        || strategyName == GroupStrategy::kModifiedTime
        || strategyName == GroupStrategy::kCreatedTime
        || strategyName == GroupStrategy::kCustomPath
        || strategyName == GroupStrategy::kCustomTime) {
        fmDebug() << "GroupingEngine: Fast path for built-in strategy" << strategyName
                  << "- skipping file info conversion";
        return !groupFiles.isEmpty();
    }

    // For NoGroupStrategy, it always returns false (groups are not visible in no-group mode)
    if (strategyName == GroupStrategy::kNoGroup) {
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
    if (!m_visibleTreeChildren || !m_childrenDataMap || file.isNull()) {
        return {};   // 如果初始节点未展开或数据结构无效，则其下没有任何“展开的文件”
    }

    bool isExpanded = file->data(ItemRoles::kItemTreeViewExpandedRole).toBool();
    if (!isExpanded) {
        return {};
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
        // Check for cancellation during tree traversal
        if (shouldCancel()) {
            fmDebug() << "GroupingEngine: Tree traversal canceled by user";
            return expandedFiles;   // Return what we have collected so far
        }

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

QUrl GroupingEngine::findTopLevelAncestorOf(const QUrl &anchorUrl) const
{
    // --- 1. 前置安全与边界检查 ---
    if (!m_visibleTreeChildren || m_visibleTreeChildren->isEmpty() || !anchorUrl.isValid()) {
        return QUrl();   // 数据无效
    }

    // 如果 anchorUrl 就是根节点本身，它没有顶层祖先
    if (anchorUrl == m_rootUrl) {
        return QUrl();
    }

    // 检查 anchorUrl 是否本身就是顶层目录
    const QList<QUrl> &rootChildren = m_visibleTreeChildren->value(m_rootUrl);
    if (rootChildren.contains(anchorUrl)) {
        return anchorUrl;
    }

    // --- 2. 构建子到父的反向映射表 ---
    QHash<QUrl, QUrl> childToParentMap;
    for (auto it = m_visibleTreeChildren->constBegin(); it != m_visibleTreeChildren->constEnd(); ++it) {
        const QUrl &parent = it.key();
        const QList<QUrl> &children = it.value();
        for (const QUrl &child : children) {
            childToParentMap.insert(child, parent);
        }
    }

    // --- 3. 从 anchorUrl 向上回溯 ---
    QUrl currentUrl = anchorUrl;
    int safetyCounter = 0;   // 防止因数据错误导致无限循环
    const int maxDepth = 1000;   // 设定一个合理的最大深度

    while (safetyCounter++ < maxDepth) {
        // 获取当前节点的父节点
        const QUrl parentUrl = childToParentMap.value(currentUrl);

        // 检查终止条件
        if (!parentUrl.isValid()) {
            // 到达了一个没有父节点的节点（但它不是根），说明 anchorUrl 不在 m_rootUrl 的子树下
            fmWarning() << "Could not trace back to root. Anchor URL might not be in the tree:" << anchorUrl;
            return QUrl();   // 查找失败
        }

        if (parentUrl == m_rootUrl) {
            // 找到了！当前节点的父节点是根，所以当前节点就是我们要找的顶层祖先
            return currentUrl;
        }

        // 继续向上移动
        currentUrl = parentUrl;
    }

    fmWarning() << "Search for top-level ancestor exceeded max depth. Possible cyclic dependency in data.";
    return QUrl();   // 查找失败（超出最大深度）
}

FileInfoPointer GroupingEngine::getFileInfoFromFileItem(const FileItemDataPointer &file) const
{
    if (!file) {
        return nullptr;
    }

    auto fileInfo = file->fileInfo();
    const auto url = file->data(ItemRoles::kItemUrlRole).toUrl();

    if (!fileInfo) {
        fileInfo = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
        Q_ASSERT(fileInfo);
        if (!fileInfo) {
            fmWarning() << "GroupingEngine: Invalid file info for" << file->data(DFMBASE_NAMESPACE::Global::kItemUrlRole).toUrl();
        }
    }
    return fileInfo;
}

DPWORKSPACE_END_NAMESPACE
