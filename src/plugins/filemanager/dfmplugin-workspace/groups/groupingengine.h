// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GROUPINGENGINE_H
#define GROUPINGENGINE_H

#include "dfmplugin_workspace_global.h"
#include "groups/filegroupdata.h"
#include "groups/groupedmodeldata.h"

#include <dfm-base/interfaces/abstractgroupstrategy.h>

#include <QObject>
#include <QHash>
#include <QString>
#include <QList>

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
    struct GroupingResult
    {
        QList<FileGroupData> groups;   ///< The grouped file data
        bool success = false;   ///< Whether the operation succeeded
        QString errorMessage;   ///< Error message if operation failed
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
                              DFMBASE_NAMESPACE::AbstractGroupStrategy *strategy) const;

    /**
     * @brief Reorder existing groups in GroupedModelData according to current group order
     * @param modelData Pointer to the GroupedModelData to reorder
     */
    void reorderGroups(GroupedModelData *modelData) const;

    /**
     * @brief Generate flattened model data from grouping result
     * @param groupingResult The result from groupFiles()
     * @param expansionStates Current expansion states for groups
     * @return The flattened model data ready for use in views
     */
    GroupedModelData generateModelData(const GroupingResult &groupingResult,
                                       const QHash<QString, bool> &expansionStates) const;

    /**
     * @brief Set the current group order
     * @param order The sort order for groups
     */
    void setGroupOrder(Qt::SortOrder order);

private:
    /**
     * @brief Perform the actual grouping algorithm
     * @param files List of files to group
     * @param strategy The grouping strategy
     * @return The grouping result
     */
    GroupingResult performGrouping(const QList<FileItemDataPointer> &files,
                                   DFMBASE_NAMESPACE::AbstractGroupStrategy *strategy) const;

    /**
     * @brief Sort groups by their display order
     * @param groups List of groups to sort
     * @param strategy The grouping strategy
     */
    void sortGroupsByDisplayOrder(QList<FileGroupData> &groups) const;

    /**
     * @brief Check if a group is visible with file info conversion
     * @param groupKey The group key
     * @param groupFiles The list of files in the group
     * @param strategy The grouping strategy
     * @return true if the group should be visible, false otherwise
     */
    bool isGroupVisibleWithConversion(const QString &groupKey,
                                      const QList<FileItemDataPointer> &groupFiles,
                                      DFMBASE_NAMESPACE::AbstractGroupStrategy *strategy) const;

    // Configuration
    Qt::SortOrder m_groupOrder = Qt::AscendingOrder;
};

DPWORKSPACE_END_NAMESPACE

#endif   // GROUPINGENGINE_H
