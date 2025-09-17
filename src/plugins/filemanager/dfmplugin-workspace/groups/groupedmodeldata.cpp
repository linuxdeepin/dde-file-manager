// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "groupedmodeldata.h"

#include <QMutexLocker>
#include <algorithm>

DPWORKSPACE_BEGIN_NAMESPACE

GroupedModelData::GroupedModelData()
{
}

GroupedModelData::GroupedModelData(const GroupedModelData &other)
{
    QMutexLocker locker(&other.m_mutex);
    groups = other.groups;
    flattenedItems = other.flattenedItems;
    groupExpansionStates = other.groupExpansionStates;
}

GroupedModelData &GroupedModelData::operator=(const GroupedModelData &other)
{
    if (this != &other) {
        QMutexLocker locker1(&m_mutex);
        QMutexLocker locker2(&other.m_mutex);

        groups = other.groups;
        flattenedItems = other.flattenedItems;
        groupExpansionStates = other.groupExpansionStates;
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

void GroupedModelData::rebuildFlattenedItems()
{
    QMutexLocker locker(&m_mutex);

    flattenedItems.clear();

    int index = 0;
    for (auto &group : groups) {
        group.displayInedx = index;
        // Always add the group header
        flattenedItems.append(ModelItemWrapper(&group));

        // Add files if the group is expanded
        bool isExpanded = groupExpansionStates.value(group.groupKey, true);
        if (isExpanded) {
            for (const auto &file : group.files) {
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
    groups.clear();
    flattenedItems.clear();
    groupExpansionStates.clear();
}

bool GroupedModelData::isEmpty() const
{
    return groups.isEmpty();
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

DPWORKSPACE_END_NAMESPACE
