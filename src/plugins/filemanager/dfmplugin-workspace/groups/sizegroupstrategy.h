 // SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIZEGROUPSTRATEGY_H
#define SIZEGROUPSTRATEGY_H

#include "abstractgroupstrategy.h"

#include <dfm-base/dfm_global_defines.h>

DPWORKSPACE_BEGIN_NAMESPACE

/**
 * @brief Size-based grouping strategy implementation
 * 
 * This strategy groups files based on their file sizes,
 * organizing them into categories like "Empty", "Small", "Large", etc.
 */
class SizeGroupStrategy : public AbstractGroupStrategy
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent object
     */
    explicit SizeGroupStrategy(QObject *parent = nullptr);

    /**
     * @brief Destructor
     */
    ~SizeGroupStrategy() override;

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
     * @brief Classify file size into a group
     * @param size The file size in bytes
     * @return The corresponding size group key
     */
    QString classifyBySize(qint64 size) const;

    /**
     * @brief Size order according to requirements (from smallest to largest)
     */
    static const QStringList SIZE_ORDER;

    /**
     * @brief Display names for each size group with size ranges
     */
    static const QHash<QString, QString> DISPLAY_NAMES;
};

DPWORKSPACE_END_NAMESPACE

#endif // SIZEGROUPSTRATEGY_H