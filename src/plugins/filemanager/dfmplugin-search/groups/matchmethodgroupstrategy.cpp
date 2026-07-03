// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "matchmethodgroupstrategy.h"

#include <dfm-base/dfm_log_defines.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/sortfileinfo.h>

#include <dfm-search/dsearch_global.h>

DPSEARCH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

MatchMethodGroupStrategy::MatchMethodGroupStrategy(QObject *parent)
    : AbstractGroupStrategy(parent)
{
    fmDebug() << "MatchMethodGroupStrategy: initialized";
}

MatchMethodGroupStrategy::~MatchMethodGroupStrategy()
{
}

QString MatchMethodGroupStrategy::getGroupKey(const FileInfoPointer &info) const
{
    Q_UNUSED(info)
    // No SortInfo carried (non-search context, or sortInfo null) → degrade to
    // Exact. The strategy only appears in the search-scheme menu, so this path
    // is a defensive fallback rather than a normal case.
    return MatchMethod::kGroupExact;
}

QString MatchMethodGroupStrategy::getGroupKey(const FileInfoPointer &info, const SortInfoPointer &sortInfo) const
{
    Q_UNUSED(info)
    // sortInfo is emitted once by SearchDirIterator and never mutated
    // afterwards — reading searchType() is a plain int load, lock-free and
    // race-free. Semantic = 40 → Smart; everything else (FileName/Content/Ocr)
    // → Exact. Priority (Exact > Smart) is already applied upstream by
    // mergeResults' score-based dedup, so a URL that matched both ways shows
    // up here only with the surviving (higher-priority) type.
    if (sortInfo) {
        const int type = sortInfo->searchType();
        if (type == static_cast<int>(DFMSEARCH::SearchType::Semantic))
            return MatchMethod::kGroupSmart;
    }
    return MatchMethod::kGroupExact;
}

QString MatchMethodGroupStrategy::getGroupDisplayName(const QString &groupKey) const
{
    if (groupKey == MatchMethod::kGroupExact)
        return tr("Exact Match");
    if (groupKey == MatchMethod::kGroupSmart)
        return tr("Smart Match");
    return groupKey;
}

QStringList MatchMethodGroupStrategy::getGroupOrder() const
{
    return { MatchMethod::kGroupExact, MatchMethod::kGroupSmart };
}

int MatchMethodGroupStrategy::getGroupDisplayOrder(const QString &groupKey) const
{
    if (groupKey == MatchMethod::kGroupExact)
        return 0;
    if (groupKey == MatchMethod::kGroupSmart)
        return 1;
    return 2;   // unknown → end
}

bool MatchMethodGroupStrategy::isGroupVisible(const QString &groupKey, const QList<FileInfoPointer> &infos) const
{
    Q_UNUSED(groupKey)
    // Same pattern as TypeGroupStrategy — hide empty groups so a search with
    // no semantic results shows no "Smart" header.
    return !infos.isEmpty();
}

QString MatchMethodGroupStrategy::getStrategyName() const
{
    return MatchMethod::kStrategyName;
}
