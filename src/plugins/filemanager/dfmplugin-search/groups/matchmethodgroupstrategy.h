// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MATCHMETHODGROUPSTRATEGY_H
#define MATCHMETHODGROUPSTRATEGY_H

#include "dfmplugin_search_global.h"

#include <dfm-base/interfaces/abstractgroupstrategy.h>
#include <dfm-base/dfm_global_defines.h>

DPSEARCH_BEGIN_NAMESPACE

// Groups search results by how each URL was matched (Exact vs Smart). Pure
// search business: lives in dfmplugin_search, crosses the plugin boundary to
// the workspace only through the dfm-base AbstractGroupStrategy abstract
// pointer. The match method is read directly from each item's SortFileInfo
// (searchType()), which the SearchDirIterator stamps once before emitting —
// so the strategy is stateless and needs no shared registry / winId / locking.
// Priority (Exact > Smart) is already resolved upstream by
// SimplifiedSearchWorker::mergeResults' score-based dedup.
class MatchMethodGroupStrategy : public DFMBASE_NAMESPACE::AbstractGroupStrategy
{
    Q_OBJECT

public:
    explicit MatchMethodGroupStrategy(QObject *parent);
    ~MatchMethodGroupStrategy() override;

    // Basic overload (non-search / no SortInfo): default to Exact.
    QString getGroupKey(const FileInfoPointer &info) const override;
    // SortInfo overload (search view): reads searchType() to classify.
    QString getGroupKey(const FileInfoPointer &info, const SortInfoPointer &sortInfo) const override;
    QString getGroupDisplayName(const QString &groupKey) const override;
    QStringList getGroupOrder() const override;
    int getGroupDisplayOrder(const QString &groupKey) const override;
    bool isGroupVisible(const QString &groupKey, const QList<FileInfoPointer> &infos) const override;
    QString getStrategyName() const override;
    bool isTruncationEnabled() const override;
};

DPSEARCH_END_NAMESPACE

#endif   // MATCHMETHODGROUPSTRATEGY_H
