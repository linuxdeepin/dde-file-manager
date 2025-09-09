// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOGROUPSTRATEGY_H
#define NOGROUPSTRATEGY_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/interfaces/abstractgroupstrategy.h>
#include <dfm-base/dfm_global_defines.h>

DFMBASE_USE_NAMESPACE
DPWORKSPACE_BEGIN_NAMESPACE

/**
 * @brief No grouping strategy implementation
 *
 * This strategy represents the "no grouping" mode where all files
 * are treated as a single virtual group. This is the default mode
 * and is also used for testing the grouping framework.
 */
class NoGroupStrategy : public DFMBASE_NAMESPACE::AbstractGroupStrategy
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent object
     */
    explicit NoGroupStrategy(QObject *parent = nullptr);

    /**
     * @brief Destructor
     */
    ~NoGroupStrategy() override;

    // AbstractGroupStrategy interface implementation
    QString getGroupKey(const FileInfoPointer &info) const override;
    QString getGroupDisplayName(const QString &groupKey) const override;
    QStringList getGroupOrder(Qt::SortOrder order = Qt::AscendingOrder) const override;
    int getGroupDisplayOrder(const QString &groupKey, Qt::SortOrder order = Qt::AscendingOrder) const override;
    bool isGroupVisible(const QString &groupKey, const QList<FileInfoPointer> &infos) const override;
    QString getStrategyName() const override;
    DFMBASE_NAMESPACE::Global::ItemRoles getCorrespondingRole() const override;

private:
    static constexpr const char *kNoGroupKey = "no-group";
};

DPWORKSPACE_END_NAMESPACE

#endif   // NOGROUPSTRATEGY_H
