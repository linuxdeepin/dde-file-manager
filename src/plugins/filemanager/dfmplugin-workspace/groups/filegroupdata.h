// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEGROUPDATA_H
#define FILEGROUPDATA_H

#include "dfmplugin_workspace_global.h"
#include "models/fileitemdata.h"

#include <QUrl>
#include <QString>
#include <QList>
#include <functional>

DPWORKSPACE_BEGIN_NAMESPACE

/**
 * @brief Data container for a single file group
 * 
 * This class stores information about a group of files, including
 * the group metadata and the list of files in the group.
 */
class FileGroupData
{
public:
    /**
     * @brief Default constructor
     */
    FileGroupData();

    /**
     * @brief Copy constructor
     */
    FileGroupData(const FileGroupData &other);

    /**
     * @brief Assignment operator
     */
    FileGroupData &operator=(const FileGroupData &other);

    /**
     * @brief Destructor
     */
    ~FileGroupData();

    // Group metadata
    QString groupKey;           ///< Internal group identifier
    QString displayName;        ///< Display name for the group
    int fileCount;             ///< Number of files in the group
    bool isExpanded;           ///< Expansion state (default: true)
    int displayOrder;          ///< Display order for sorting groups
    int displayInedx;          ///< Display index for sorting groups   
    QList<FileItemDataPointer> files; ///< List of files in this group

    /**
     * @brief Get the header text for display
     * @return Formatted string "Group Name (file count)"
     */
    QString getHeaderText() const;

    /**
     * @brief Add a file to this group
     * @param file The file item to add
     */
    void addFile(const FileItemDataPointer &file);

    /**
     * @brief Remove a file from this group
     * @param url The URL of the file to remove
     * @return True if the file was found and removed, false otherwise
     */
    bool removeFile(const QUrl &url);

    /**
     * @brief Clear all files from this group
     */
    void clear();

    /**
     * @brief Check if the group is empty
     * @return True if the group has no files, false otherwise
     */
    bool isEmpty() const;

    /**
     * @brief Sort files within this group
     * @param lessThan Comparison function for sorting
     */
    void sortFiles(const std::function<bool(const FileItemDataPointer&, const FileItemDataPointer&)> &lessThan);

    /**
     * @brief Update file count based on current files list
     */
    void updateFileCount();
};

DPWORKSPACE_END_NAMESPACE

#endif // FILEGROUPDATA_H 