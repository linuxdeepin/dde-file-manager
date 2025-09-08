// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIMEGROUPSTRATEGY_H
#define TIMEGROUPSTRATEGY_H

#include "abstractgroupstrategy.h"

#include <dfm-base/dfm_global_defines.h>

#include <QDateTime>

DPWORKSPACE_BEGIN_NAMESPACE

/**
 * @brief Time-based grouping strategy implementation
 * 
 * This strategy groups files based on their modification or creation time,
 * organizing them into time-based categories like "Today", "Yesterday", etc.
 */
class TimeGroupStrategy : public AbstractGroupStrategy
{
    Q_OBJECT

public:
    /**
     * @brief Time type for grouping
     */
    enum TimeType {
        ModificationTime,   ///< Use file modification time
        CreationTime       ///< Use file creation time
    };

    /**
     * @brief Constructor
     * @param timeType The type of time to use for grouping
     * @param parent Parent object
     */
    explicit TimeGroupStrategy(TimeType timeType = ModificationTime, QObject *parent = nullptr);

    /**
     * @brief Destructor
     */
    ~TimeGroupStrategy() override;

    // AbstractGroupStrategy interface implementation
    QString getGroupKey(const FileItemDataPointer &item) const override;
    QString getGroupDisplayName(const QString &groupKey) const override;
    QStringList getGroupOrder(Qt::SortOrder order = Qt::AscendingOrder) const override;
    int getGroupDisplayOrder(const QString &groupKey, Qt::SortOrder order = Qt::AscendingOrder) const override;
    bool isGroupVisible(const QString &groupKey, const QList<FileItemDataPointer> &items) const override;
    QString getStrategyName() const override;
    DFMBASE_NAMESPACE::Global::ItemRoles getCorrespondingRole() const override;

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
     * @brief Basic time order according to requirements (most recent first)
     */
    static const QStringList TIME_ORDER;

    /**
     * @brief Display names for basic time groups
     */
    static const QHash<QString, QString> DISPLAY_NAMES;
};

DPWORKSPACE_END_NAMESPACE

#endif // TIMEGROUPSTRATEGY_H 