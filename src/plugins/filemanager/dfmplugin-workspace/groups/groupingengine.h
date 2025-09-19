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
    enum class UpdateMode {
        kNoGrouping,
        kInsert,
        kRemove
    };

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
     * @brief Result structure for update operations
     */
    struct UpdateResult
    {
        GroupedModelData newData;   ///< The new model data
        int pos;   ///< The position of the updated items
        int count;   ///< The number of items in the update
        bool success = false;   ///< Whether the operation succeeded
        bool alwaysUpdate = false;
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
     * @brief Remove files from the model data
     * @return The updated model data
     * @param oldData The old model data
     */
    UpdateResult removeFilesFromModelData(const GroupedModelData &oldData);

    /**
     * @brief Insert files into the model data
     * @param anchorUrl The anchor URL for the insertion
     * @param oldData The old model data
     * @param strategy The grouping strategy to use
     * @return The updated model data
     */
    UpdateResult insertFilesToModelData(const QUrl &anchorUrl,
                                        const GroupedModelData &oldData,
                                        DFMBASE_NAMESPACE::AbstractGroupStrategy *strategy);

    /**
     * @brief Group files using the specified strategy
     * @param files List of files to group
     * @param strategy The grouping strategy to use
     * @return The grouping result
     */
    GroupingResult groupFiles(const QList<FileItemDataPointer> &files,
                              DFMBASE_NAMESPACE::AbstractGroupStrategy *strategy) const;

    /**
     * @brief Get the current update mode
     */
    UpdateMode currentUpdateMode() const;

    /**
     * @brief Get the current visible children for update
     */
    QPair<int, int> currentUpdateChildrenRange() const;

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
     * @brief Find the preceding anchor URL for a given slice range
     * @param container The container to search in
     * @param sliceRange The slice range to search
     * @return The preceding anchor URL, or std::nullopt if none found
     */
    std::optional<QUrl> findPrecedingAnchor(const QList<QUrl> &container, const QPair<int, int> &sliceRange);

    /**
     * @brief Set the current group order
     * @param order The sort order for groups
     */
    void setGroupOrder(Qt::SortOrder order);

    /**
     * @brief Set the current visible tree children
     * @param children The map of URLs to child URLs
     */
    void setVisibleTreeChildren(QHash<QUrl, QList<QUrl>> *children);

    /**
     * @brief Set the children data map
     * @param map The children data map
     */
    void setChildrenDataMap(QHash<QUrl, FileItemDataPointer> *map);

    /**
     * @brief Set the current update mode
     */
    void setUpdateMode(UpdateMode mode);

    /**
     * @brief Set the current visible children for update
     */
    void setUpdateChildren(const QList<QUrl> &children);

    /**
     * @brief Set the current visible children for update
     */
    void setUpdateChildrenRange(int pos, int count);

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
     * @brief Get FileInfoPointer from FileItemDataPointer
     * @param file The file item data pointer
     * @return FileInfoPointer or nullptr if failed
     */
    FileInfoPointer getFileInfoFromFileItem(const FileItemDataPointer &file) const;

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

    /**
     * @brief Find expanded files in the file tree
     * @param file The file to start from
     * @return The list of expanded files
     */
    QList<FileItemDataPointer> findExpandedFiles(const FileItemDataPointer &file) const;

private:
    /**
     * @brief Collect files to be inserted from visible children
     * @param filesToInsert Output list of files to insert
     * @return true if successful, false otherwise
     */
    bool collectFilesToInsert(QList<FileItemDataPointer> &filesToInsert) const;

    /**
     * @brief Process files and update groups in the model data
     * @param filesToInsert List of files to insert
     * @param strategy The grouping strategy to use
     * @param newData The model data to update
     * @param updatedGroups Set to track which groups were updated
     * @param groupAdded Output parameter indicating if new groups were added
     * @param alwaysUpdate Output parameter indicating if always update is needed
     * @return true if successful, false otherwise
     */
    bool processFilesAndUpdateGroups(const QList<FileItemDataPointer> &filesToInsert,
                                     DFMBASE_NAMESPACE::AbstractGroupStrategy *strategy,
                                     GroupedModelData *newData,
                                     QSet<QString> *updatedGroups,
                                     bool *groupAdded,
                                     bool *alwaysUpdate) const;

    /**
     * @brief Calculate the insertion position in the model data
     * @param anchorUrl The anchor URL for the insertion
     * @param newData The model data
     * @param updatedGroups The set of updated groups
     * @param groupAdded Whether new groups were added
     * @param pos Output parameter for the calculated position
     * @return true if successful, false otherwise
     */
    bool calculateInsertPosition(const QUrl &anchorUrl,
                                 const GroupedModelData &newData,
                                 const QSet<QString> &updatedGroups,
                                 bool groupAdded,
                                 int *pos) const;

    /**
     * @brief Finalize the model data update by inserting files at the correct position
     * @param filesToInsert List of files to insert
     * @param strategy The grouping strategy to use
     * @param newData The model data to update
     * @param updatedGroups The set of updated groups
     * @param groupAdded Whether new groups were added
     * @param pos The position to insert files
     * @return true if successful, false otherwise
     */
    bool finalizeModelUpdate(const QList<FileItemDataPointer> &filesToInsert,
                             DFMBASE_NAMESPACE::AbstractGroupStrategy *strategy,
                             GroupedModelData *newData,
                             const QSet<QString> &updatedGroups,
                             bool groupAdded,
                             int pos) const;

private:
    // Configuration
    Qt::SortOrder m_groupOrder = Qt::AscendingOrder;
    const QHash<QUrl, QList<QUrl>> *m_visibleTreeChildren { nullptr };
    const QHash<QUrl, FileItemDataPointer> *m_childrenDataMap { nullptr };
    // for update
    UpdateMode m_updateMode { UpdateMode::kNoGrouping };
    QList<QUrl> m_visibleChildrenForUpdate;
    QPair<int, int> m_visibleChildrenRangeForUpdate;
};

DPWORKSPACE_END_NAMESPACE

#endif   // GROUPINGENGINE_H
