// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "groupedmodeldata.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/universalutils.h>

#include <QMutexLocker>

#include <algorithm>

DPWORKSPACE_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

GroupedModelData::GroupedModelData()
{
}

GroupedModelData::GroupedModelData(const GroupedModelData &other)
{
    QMutexLocker locker(&other.m_mutex);
    groups = other.groups;
    flattenedItems = other.flattenedItems;
    groupExpansionStates = other.groupExpansionStates;
    truncationStates = other.truncationStates;
    m_truncationEnabled = other.m_truncationEnabled;
}

GroupedModelData &GroupedModelData::operator=(const GroupedModelData &other)
{
    if (this != &other) {
        QMutexLocker locker1(&m_mutex);
        QMutexLocker locker2(&other.m_mutex);

        groups = other.groups;
        flattenedItems = other.flattenedItems;
        groupExpansionStates = other.groupExpansionStates;
        truncationStates = other.truncationStates;
        m_truncationEnabled = other.m_truncationEnabled;
    }
    return *this;
}

GroupedModelData::~GroupedModelData()
{
}

QList<FileItemDataPointer> GroupedModelData::getAllFiles() const
{
    QList<FileItemDataPointer> allFiles;
    for (const auto &group : groups) {
        for (const auto &file : group.files) {
            if (file) {
                allFiles.append(file);
            }
        }
    }
    return allFiles;
}

void GroupedModelData::setGroupExpanded(const QString &groupKey, bool expanded)
{
    if (groupKey.isEmpty()) {
        return;
    }

    groupExpansionStates[groupKey] = expanded;

    // Update the corresponding group's expansion state
    for (auto &group : groups) {
        if (group.groupKey == groupKey) {
            group.isExpanded = expanded;
            break;
        }
    }

    rebuildFlattenedItems();
}

bool GroupedModelData::isGroupExpanded(const QString &groupKey) const
{
    return groupExpansionStates.value(groupKey, true);   // Default to expanded
}

void GroupedModelData::setGroupTruncated(const QString &groupKey, bool truncated, bool rebuild)
{
    if (groupKey.isEmpty() || !m_truncationEnabled) {
        return;
    }

    if (truncationStates.value(groupKey, false) == truncated) {
        return;
    }

    truncationStates[groupKey] = truncated;
    if (rebuild) {
        rebuildFlattenedItems();
    }
}

bool GroupedModelData::isGroupTruncated(const QString &groupKey) const
{
    return truncationStates.value(groupKey, false);
}

bool GroupedModelData::isGroupTruncatedInitialized(const QString &groupKey) const
{
    return truncationStates.contains(groupKey);
}

int GroupedModelData::getVisibleFileCount(const QString &groupKey) const
{
    const FileGroupData *group = getGroup(groupKey);
    if (!group || !isGroupExpanded(groupKey)) {
        return 0;
    }

    if (!m_truncationEnabled || !isGroupTruncated(groupKey)) {
        return group->fileCount;
    }

    return qMin(group->fileCount, kGroupTruncateLimit);
}

void GroupedModelData::setTruncationEnabled(bool enabled)
{
    m_truncationEnabled = enabled;
}

bool GroupedModelData::isTruncationEnabled() const
{
    return m_truncationEnabled;
}

void GroupedModelData::updateGroupHeader(const QString &groupKey)
{
    if (groupKey.isEmpty()) {
        return;
    }

    QMutexLocker locker(&m_mutex);

    // Find the group data
    const FileGroupData *groupData = nullptr;
    for (auto &group : std::as_const(groups)) {
        if (group.groupKey == groupKey) {
            groupData = &group;
            break;
        }
    }

    if (!groupData) {
        return;
    }

    // Find and update the group header in flattenedItems
    for (auto &item : flattenedItems) {
        if (item.isGroupHeader() && item.groupKey == groupKey) {
            // Create a new ModelItemWrapper with updated group data
            item = ModelItemWrapper(groupData, isGroupTruncated(groupKey), m_truncationEnabled);
            break;
        }
    }
}

void GroupedModelData::rebuildFlattenedItems()
{
    QMutexLocker locker(&m_mutex);

    flattenedItems.clear();

    int index = 0;
    for (auto &group : groups) {
        group.displayIndex = index;
        group.isExpanded = groupExpansionStates.value(group.groupKey, true);
        // Always add the group header
        flattenedItems.append(ModelItemWrapper(&group, isGroupTruncated(group.groupKey), m_truncationEnabled));

        // Add files if the group is expanded
        if (group.isExpanded) {
            const int visibleCount = getVisibleFileCount(group.groupKey);
            for (int fileIndex = 0; fileIndex < visibleCount; ++fileIndex) {
                const auto &file = group.files.at(fileIndex);
                if (file) {
                    file->setGroupDisplayIndex(index);
                    flattenedItems.append(ModelItemWrapper(file, group.groupKey));
                }
            }
        }
        ++index;
    }
}

void GroupedModelData::clear()
{
    QMutexLocker locker(&m_mutex);

    groups.clear();
    flattenedItems.clear();
    groupExpansionStates.clear();
    truncationStates.clear();
    m_truncationEnabled = false;
}

bool GroupedModelData::isEmpty() const
{
    return groups.isEmpty();
}

bool GroupedModelData::addGroup(const FileGroupData &group)
{
    if (group.groupKey.isEmpty()) {
        return false;
    }

    // Check if a group with the same key already exists
    for (auto &existingGroup : std::as_const(groups)) {
        if (existingGroup.groupKey == group.groupKey) {
            return false;   // Group with this key already exists
        }
    }

    // Add the new group
    groups.append(group);

    // Ensure the expansion state is consistent
    if (!groupExpansionStates.contains(group.groupKey)) {
        groupExpansionStates[group.groupKey] = group.isExpanded;
    }

    return true;
}

bool GroupedModelData::removeGroup(const QString &groupKey)
{
    if (groupKey.isEmpty()) {
        return false;
    }

    // Find and remove the group
    for (auto it = groups.begin(); it != groups.end(); ++it) {
        if (it->groupKey == groupKey) {
            groups.erase(it);
            groupExpansionStates.remove(groupKey);
            truncationStates.remove(groupKey);
            return true;
        }
    }

    return false;   // Group not found
}

void GroupedModelData::insertItem(int index, const ModelItemWrapper &item)
{
    QMutexLocker locker(&m_mutex);

    // Make sure the index is within valid range
    if (index < 0 || index > flattenedItems.size()) {
        return;
    }

    flattenedItems.insert(index, item);
}

int GroupedModelData::removeItems(int index, int count)
{
    QMutexLocker locker(&m_mutex);

    // Make sure the parameters are within valid range
    if (index < 0 || index >= flattenedItems.size() || count <= 0) {
        return 0;
    }

    // Adjust count if it exceeds the available items
    int actualCount = qMin(count, flattenedItems.size() - index);

    // Remove the items
    flattenedItems.erase(flattenedItems.begin() + index, flattenedItems.begin() + index + actualCount);

    return actualCount;
}

void GroupedModelData::replaceItem(int index, const ModelItemWrapper &item)
{
    QMutexLocker locker(&m_mutex);

    // Make sure the index is within valid range
    if (index < 0 || index >= flattenedItems.size()) {
        return;
    }

    // Replace the item
    flattenedItems[index] = item;
}

FileGroupData *GroupedModelData::getGroup(const QString &groupKey)
{
    auto it = std::find_if(groups.begin(), groups.end(),
                           [&groupKey](const FileGroupData &group) {
                               return group.groupKey == groupKey;
                           });

    return (it != groups.end()) ? &(*it) : nullptr;
}

const FileGroupData *GroupedModelData::getGroup(const QString &groupKey) const
{
    auto it = std::find_if(groups.constBegin(), groups.constEnd(),
                           [&groupKey](const FileGroupData &group) {
                               return group.groupKey == groupKey;
                           });

    return (it != groups.constEnd()) ? &(*it) : nullptr;
}

int GroupedModelData::getItemCount() const
{
    QMutexLocker locker(&m_mutex);
    return flattenedItems.size();
}

int GroupedModelData::getFileItemCount() const
{
    QMutexLocker locker(&m_mutex);
    int count = 0;
    for (const auto &group : groups) {
        count += group.files.size();
    }
    return count;
}

int GroupedModelData::getGroupItemCount() const
{
    QMutexLocker locker(&m_mutex);
    return groups.size();
}

ModelItemWrapper GroupedModelData::getItemAt(int index) const
{
    QMutexLocker locker(&m_mutex);
    if (index < 0 || index >= flattenedItems.size()) {
        return ModelItemWrapper();   // Return invalid wrapper
    }
    return flattenedItems.at(index);
}

std::optional<int> GroupedModelData::findGroupHeaderStartPos(const QString &key) const
{
    QMutexLocker locker(&m_mutex);

    for (int i = 0; i < flattenedItems.size(); ++i) {
        const ModelItemWrapper &item = flattenedItems.at(i);
        if (item.isGroupHeader() && item.groupKey == key) {
            return i;
        }
    }

    return std::nullopt;
}

std::optional<int> GroupedModelData::findFileStartPos(const QUrl &url) const
{
    QMutexLocker locker(&m_mutex);

    if (!url.isValid()) {
        return std::nullopt;
    }

    for (int i = 0; i < flattenedItems.size(); ++i) {
        const ModelItemWrapper &item = flattenedItems.at(i);
        if (item.isFileItem() && item.fileData) {
            const auto &fileUrl = item.fileData->data(kItemUrlRole).toUrl();
            if (UniversalUtils::urlEquals(fileUrl, url)) {
                return i;
            }
        }
    }

    return std::nullopt;
}

DPWORKSPACE_END_NAMESPACE
