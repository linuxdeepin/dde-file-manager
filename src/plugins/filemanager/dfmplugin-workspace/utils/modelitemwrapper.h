// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MODELITEMWRAPPER_H
#define MODELITEMWRAPPER_H

#include "dfmplugin_workspace_global.h"
#include "models/fileitemdata.h"

#include <dfm-base/dfm_global_defines.h>

#include <QUrl>
#include <QString>
#include <QVariant>

DPWORKSPACE_BEGIN_NAMESPACE

class FileGroupData;

/**
 * @brief Wrapper for model items (file items or group header items)
 *
 * This class unifies the handling of different types of items in the model,
 * allowing the same interface to work with both file items and group headers.
 */
class ModelItemWrapper
{
public:
    /**
     * @brief Type of the wrapped item
     */
    enum ItemType {
        FileItem,   ///< Regular file item
        GroupHeaderItem   ///< Group header item
    };

    /**
     * @brief Default constructor
     */
    ModelItemWrapper();

    /**
     * @brief Constructor for file item
     * @param fileData The file data pointer
     * @param groupKey The group this file belongs to
     */
    ModelItemWrapper(const FileItemDataPointer &fileData, const QString &groupKey);

    /**
     * @brief Constructor for group header item
     * @param groupData The group data pointer
     */
    ModelItemWrapper(const FileGroupData *groupData);

    /**
     * @brief Copy constructor
     */
    ModelItemWrapper(const ModelItemWrapper &other);

    /**
     * @brief Assignment operator
     */
    ModelItemWrapper &operator=(const ModelItemWrapper &other);

    /**
     * @brief Destructor
     */
    ~ModelItemWrapper();

    // Member variables
    ItemType itemType;   ///< Type of the item
    QString groupKey;   ///< Group identifier
    FileItemDataPointer fileData;   ///< File data (valid for FileItem)
    const FileGroupData *groupData;   ///< Group data pointer (valid for GroupHeaderItem)

    /**
     * @brief Check if this is a group header item
     * @return True if this is a group header, false otherwise
     */
    bool isGroupHeader() const;

    /**
     * @brief Check if this is a file item
     * @return True if this is a file item, false otherwise
     */
    bool isFileItem() const;

    /**
     * @brief Get data for a specific role
     * @param role The data role to retrieve
     * @return The data for the specified role
     */
    QVariant getData(int role) const;

    /**
     * @brief Check if the wrapper is valid
     * @return True if the wrapper contains valid data, false otherwise
     */
    bool isValid() const;
};

DPWORKSPACE_END_NAMESPACE

#endif   // MODELITEMWRAPPER_H
