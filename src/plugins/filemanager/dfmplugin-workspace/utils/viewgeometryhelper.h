// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIEWGEOMETRYHELPER_H
#define VIEWGEOMETRYHELPER_H

#include "dfmplugin_workspace_global.h"

#include <QObject>
#include <QRect>
#include <QModelIndex>
#include <QPair>
#include <QList>

namespace dfmplugin_workspace {

class FileView;

// Pure geometry calculations for selection ranges and item rects.
// Stateless: all methods operate on FileView public state via `view`.
class ViewGeometryHelper : public QObject
{
    Q_OBJECT
public:
    using RandeIndex = QPair<int, int>;
    using RandeIndexList = QList<RandeIndex>;

    explicit ViewGeometryHelper(FileView *parent = nullptr);

    RandeIndexList visibleIndexes(const QRect &rect) const;
    RandeIndexList rectContainsIndexes(const QRect &rect) const;
    RandeIndexList calcRectContiansIndexes(int columnCount, const QRect &rect) const;
    RandeIndexList calcGroupRectContiansIndexes(const QRect &rect) const;
    QRect calcVisualRect(int widgetWidth, int index) const;
    bool indexInRect(const QRect &actualRect, const QModelIndex &index) const;

private:
    FileView *view { nullptr };
};

}   // namespace dfmplugin_workspace

#endif   // VIEWGEOMETRYHELPER_H
