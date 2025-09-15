// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIMEGROUPSTRATEGY_H
#define TIMEGROUPSTRATEGY_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/interfaces/abstractgroupstrategy.h>
#include <dfm-base/dfm_global_defines.h>

#include <QDateTime>

DFMBASE_USE_NAMESPACE
DPWORKSPACE_BEGIN_NAMESPACE

/**
 * @brief Time-based grouping strategy implementation
 *
 * This strategy groups files based on their modification or creation time,
 * organizing them into time-based categories like "Today", "Yesterday", etc.
 */
class TimeGroupStrategy : public DFMBASE_NAMESPACE::AbstractGroupStrategy
{
    Q_OBJECT

public:
    /**
     * @brief Time type for grouping
     */
    enum TimeType {
        kModificationTime,   ///< Use file modification time
        kCreationTime,   ///< Use file creation time
        kCustomTime
    };

    /**
     * @brief Constructor
     * @param timeType The type of time to use for grouping
     * @param parent Parent object
     */
    explicit TimeGroupStrategy(TimeType timeType = kModificationTime, QObject *parent = nullptr);

    /**
     * @brief Destructor
     */
    ~TimeGroupStrategy() override;

    // AbstractGroupStrategy interface implementation
    QString getGroupKey(const FileInfoPointer &info) const override;
    QString getGroupDisplayName(const QString &groupKey) const override;
    QStringList getGroupOrder(Qt::SortOrder order = Qt::AscendingOrder) const override;
    int getGroupDisplayOrder(const QString &groupKey, Qt::SortOrder order = Qt::AscendingOrder) const override;
    bool isGroupVisible(const QString &groupKey, const QList<FileInfoPointer> &infos) const override;
    QString getStrategyName() const override;

private:
    /**
     * @brief Calculate time group for a given file time
     * @param fileTime The file timestamp
     * @return The corresponding time group key
     */
    QString calculateTimeGroup(const QDateTime &fileTime) const;

    /**
     * @brief Get display order for dynamic time groups (months, years)
     * @param groupKey The group key to get order for
     * @param order Sort order
     * @return The display order index
     */
    int getDynamicDisplayOrder(const QString &groupKey, Qt::SortOrder order) const;

    /**
     * @brief The type of time to use for grouping
     */
    TimeType m_timeType;

    /**
     * @brief Get the time order list
     * @return The basic time order according to requirements (most recent first)
     */
    static QStringList getTimeOrder();

    /**
     * @brief Get display names for time groups
     * @return Hash map of group keys to display names for basic time groups
     */
    static QHash<QString, QString> getDisplayNames();
};

DPWORKSPACE_END_NAMESPACE

#endif   // TIMEGROUPSTRATEGY_H
