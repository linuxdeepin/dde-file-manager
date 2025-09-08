// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTGROUPSTRATEGY_H
#define ABSTRACTGROUPSTRATEGY_H

#include "dfmplugin_workspace_global.h"
#include "models/fileitemdata.h"

#include <dfm-base/dfm_global_defines.h>

#include <QObject>
#include <QString>
#include <QStringList>

DPWORKSPACE_BEGIN_NAMESPACE

/**
 * @brief Abstract base class for file grouping strategies
 * 
 * This class defines the interface for all file grouping strategies.
 * Each strategy implements different ways to group files (by type, time, name, size, etc.)
 */
class AbstractGroupStrategy : public QObject
{
    Q_OBJECT

public:
    explicit AbstractGroupStrategy(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~AbstractGroupStrategy() = default;

    /**
     * @brief Get the group key for a file item
     * @param item The file item to classify
     * @return The group key string that identifies which group this file belongs to
     */
    virtual QString getGroupKey(const FileItemDataPointer &item) const = 0;

    /**
     * @brief Get the display name for a group key
     * @param groupKey The internal group key
     * @return The localized display name for the group
     */
    virtual QString getGroupDisplayName(const QString &groupKey) const = 0;

    /**
     * @brief Get the ordered list of group keys
     * @param order The sort order (ascending or descending)
     * @return List of group keys in the specified order
     */
    virtual QStringList getGroupOrder(Qt::SortOrder order = Qt::AscendingOrder) const = 0;

    /**
     * @brief Get the display order index for a group key
     * @param groupKey The group key to get order for
     * @param order The sort order (ascending or descending)
     * @return The numeric order index for sorting groups
     */
    virtual int getGroupDisplayOrder(const QString &groupKey, Qt::SortOrder order = Qt::AscendingOrder) const = 0;

    /**
     * @brief Check if a group should be visible
     * @param groupKey The group key to check
     * @param items The list of items in this group
     * @return True if the group should be displayed, false otherwise
     */
    virtual bool isGroupVisible(const QString &groupKey, const QList<FileItemDataPointer> &items) const = 0;

    /**
     * @brief Check if a group can be collapsed/expanded
     * @param groupKey The group key to check
     * @return True if the group supports collapse/expand, false otherwise
     */
    virtual bool isGroupCollapsible(const QString &groupKey) const { return true; }

    /**
     * @brief Get the strategy name for identification
     * @return The localized name of this strategy
     */
    virtual QString getStrategyName() const = 0;

    /**
     * @brief Get the corresponding ItemRole for this strategy
     * @return The ItemRole that this strategy is based on
     */
    virtual DFMBASE_NAMESPACE::Global::ItemRoles getCorrespondingRole() const = 0;
};

DPWORKSPACE_END_NAMESPACE

#endif // ABSTRACTGROUPSTRATEGY_H 