// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
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
#include <functional>

DPWORKSPACE_BEGIN_NAMESPACE

/**
 * @brief Callback function type for checking if operation should be canceled
 * @return true if the operation should be canceled, false otherwise
 */
using CancellationCheckCallback = std::function<bool()>;

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
        kRemove,
        kUpdate
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
    explicit GroupingEngine(const QUrl &rootUrl, QObject *parent = nullptr);

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
     * @brief Update files into the model data
     * @param anchorUrl The anchor URL for the insertion
     * @param oldData The old model data
     * @param strategy The grouping strategy to use
     * @return The updated model data
     */
    UpdateResult updateFilesToModelData(const QUrl &anchorUrl,
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
     * @brief Set the visible children
     * @param visibleChildren The list of visible children
     */
    void setVisibleChildren(QList<QUrl> *visibleChildren);

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

    /**
     * @brief Set the cancellation check callback
     * @param callback The callback function to check for cancellation
     */
    void setCancellationCheckCallback(CancellationCheckCallback callback);

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

    /**
     * @brief Finds the top-level ancestor directory of the given URL in the tree.
     *              * The top-level ancestor directory is defined as a direct child of the root node (m_rootUrl),
     * and an ancestor of the anchorUrl.
     *              * @param anchorUrl The URL whose top-level ancestor is to be found.
     * @return If found, returns the URL of the top-level ancestor.
     *         If anchorUrl itself is a top-level directory, returns anchorUrl.
     *         If anchorUrl is the root node or not found in the tree, returns an invalid QUrl().
     */
    QUrl findTopLevelAncestorOf(const QUrl &anchorUrl) const;

private:
    /**
     * @brief Get the group key for a set of files
     * @param filesToInsert List of files to determine group key from
     * @param anchorUrl The anchor URL
     * @param strategy The grouping strategy to use
     * @return The group key
     */
    QString getGroupKeyForFiles(const QList<FileItemDataPointer> &filesToInsert,
                                const QUrl &anchorUrl,
                                DFMBASE_NAMESPACE::AbstractGroupStrategy *strategy) const;

    /**
     * @brief Collect files to be inserted from visible children
     * @param filesToInsert Output list of files to insert
     * @return true if successful, false otherwise
     */
    bool collectFilesToInsert(QList<FileItemDataPointer> *filesToInsert) const;

    /**
     * @brief Process files and update groups in the model data
     * @param filesToInsert List of files to insert
     * @param groupKey The group key for these files
     * @param strategy The grouping strategy to use
     * @param anchorUrl The anchor URL
     * @param newData The model data to update
     * @param alwaysUpdate Output parameter indicating if always update is needed
     * @return true if successful, false otherwise
     */
    bool processFilesAndInsertGroups(const QList<FileItemDataPointer> &filesToInsert,
                                     const QString groupKey,
                                     const DFMBASE_NAMESPACE::AbstractGroupStrategy *strategy,
                                     const QUrl &anchorUrl,
                                     GroupedModelData *newData,
                                     bool *alwaysUpdate) const;

    /**
     * @brief Process files and update groups in the model data
     * @param filesToInsert List of files to insert
     * @param groupKey The group key for these files
     * @param anchorUrl The anchor URL
     * @param newData The model data to update
     * @return true if successful, false otherwise
     */
    bool processFilesAndUpdateGroups(const QList<FileItemDataPointer> &filesToInsert,
                                     const QString groupKey,
                                     const QUrl &anchorUrl,
                                     GroupedModelData *newData) const;

    /**
     * @brief Find the new anchor position in the model
     * @param oldAnchorUrl The old anchor URL
     * @param group The group to find the new anchor position in
     * @return The new anchor position, or std::nullopt if not found
     */
    std::optional<int> findNewAnchorPos(const QUrl &oldAnchorUrl, const FileGroupData *group) const;

    /**
     * @brief Check if the operation should be canceled
     * @return true if operation should be canceled, false otherwise
     */
    inline bool shouldCancel() const
    {
        return m_cancellationCheck && m_cancellationCheck();
    }

private:
    // Configuration
    QUrl m_rootUrl;
    Qt::SortOrder m_groupOrder = Qt::AscendingOrder;
    const QList<QUrl> *m_visibleChildren { nullptr };
    const QHash<QUrl, QList<QUrl>> *m_visibleTreeChildren { nullptr };
    const QHash<QUrl, FileItemDataPointer> *m_childrenDataMap { nullptr };
    // for update
    UpdateMode m_updateMode { UpdateMode::kNoGrouping };
    QList<QUrl> m_visibleChildrenForUpdate;
    QPair<int, int> m_visibleChildrenRangeForUpdate;
    // Cancellation callback
    CancellationCheckCallback m_cancellationCheck;
};

DPWORKSPACE_END_NAMESPACE

#endif   // GROUPINGENGINE_H
