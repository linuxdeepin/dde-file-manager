// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PATHGROUPSTRATEGY_H
#define PATHGROUPSTRATEGY_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/interfaces/abstractgroupstrategy.h>
#include <dfm-base/dfm_global_defines.h>

DPWORKSPACE_BEGIN_NAMESPACE

/**
 * @brief Path-based grouping strategy implementation
 *
 * This strategy groups files based on their file paths,
 * organizing them into categories like system disk, data disk, and other partitions.
 */
class PathGroupStrategy : public DFMBASE_NAMESPACE::AbstractGroupStrategy
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent object
     */
    explicit PathGroupStrategy(QObject *parent = nullptr);

    /**
     * @brief Destructor
     */
    ~PathGroupStrategy() override;

    // AbstractGroupStrategy interface implementation
    QString getGroupKey(const FileInfoPointer &info) const override;
    QString getGroupDisplayName(const QString &groupKey) const override;
    QStringList getGroupOrder() const override;
    int getGroupDisplayOrder(const QString &groupKey) const override;
    bool isGroupVisible(const QString &groupKey, const QList<FileInfoPointer> &infos) const override;
    QString getStrategyName() const override;

private:
    /**
     * @brief Classify file path into a group
     * @param path The file path
     * @return The corresponding path group key
     */
    QString classifyByPath(const QString &path) const;

    /**
     * @brief Get the path order list
     * @return The path order according to requirements
     */
    static QStringList getPathOrder();

    /**
     * @brief Get display names for path groups
     * @return Hash map of group keys to display names
     */
    static QHash<QString, QString> getDisplayNames();
    
    // 存储other组的设备信息(size和label)
    mutable QHash<QString, QVariantMap> otherGroupInfos;
};

DPWORKSPACE_END_NAMESPACE

#endif   // PATHGROUPSTRATEGY_H