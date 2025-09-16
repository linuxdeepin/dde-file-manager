// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GROUPEDMODELDATA_H
#define GROUPEDMODELDATA_H

#include "dfmplugin_workspace_global.h"

#include "groups/filegroupdata.h"
#include "groups/modelitemwrapper.h"

#include <QList>
#include <QHash>
#include <QString>
#include <QMutex>

DPWORKSPACE_BEGIN_NAMESPACE

/**
 * @brief Container for grouped model data
 *
 * This class manages the flattened representation of grouped files
 * for use in the model-view architecture.
 */
class GroupedModelData
{
public:
    /**
     * @brief Default constructor
     */
    GroupedModelData();

    /**
     * @brief Copy constructor
     */
    GroupedModelData(const GroupedModelData &other);

    /**
     * @brief Assignment operator
     */
    GroupedModelData &operator=(const GroupedModelData &other);

    /**
     * @brief Destructor
     */
    ~GroupedModelData();

    // Core data members
    QList<FileGroupData> groups;   ///< All group data
    QHash<QString, bool> groupExpansionStates;   ///< Group expansion state mapping

    /**
     * @brief Get the total number of files across all groups
     * @return Total file count
     */
    int getTotalFileCount() const;

    /**
     * @brief Get all files from all groups
     * @return List of all file data pointers
     */
    QList<FileItemDataPointer> getAllFiles() const;

    /**
     * @brief Get the item at a specific index
     * @param index The index in the flattened list
     * @return The model item wrapper at the specified index
     */
    ModelItemWrapper getItemAt(int index) const;

    /**
     * @brief Get the total number of items (groups + files)
     * @return Total item count in the flattened list
     */
    int getItemCount() const;

    /**
     * @brief Set the expansion state of a group
     * @param groupKey The group identifier
     * @param expanded True to expand, false to collapse
     */
    void setGroupExpanded(const QString &groupKey, bool expanded);

    /**
     * @brief Check if a group is expanded
     * @param groupKey The group identifier
     * @return True if the group is expanded, false otherwise
     */
    bool isGroupExpanded(const QString &groupKey) const;

    /**
     * @brief Rebuild the flattened items list
     *
     * This method reconstructs the flattened list based on current
     * groups and their expansion states.
     */
    void rebuildFlattenedItems();

    /**
     * @brief Clear all data
     */
    void clear();

    /**
     * @brief Check if the data is empty
     * @return True if there are no groups, false otherwise
     */
    bool isEmpty() const;

    /**
     * @brief Get a group by its key
     * @param groupKey The group identifier
     * @return Pointer to the group data, or nullptr if not found
     */
    FileGroupData *getGroup(const QString &groupKey);

    /**
     * @brief Get a group by its key (const version)
     * @param groupKey The group identifier
     * @return Const pointer to the group data, or nullptr if not found
     */
    const FileGroupData *getGroup(const QString &groupKey) const;

    /**
     * @brief Thread-safe access to item count
     * @return Total item count in the flattened list
     */
    int getItemCountThreadSafe() const;

    /**
     * @brief Thread-safe access to item at index
     * @param index The index in the flattened list
     * @return The model item wrapper at the specified index
     */
    ModelItemWrapper getItemAtThreadSafe(int index) const;

private:
    mutable QMutex m_mutex;   ///< Mutex for thread safety
    QList<ModelItemWrapper> flattenedItems;   ///< Flattened model items list
};

DPWORKSPACE_END_NAMESPACE

#endif   // GROUPEDMODELDATA_H
