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
    using RangeIndex = QPair<int, int>;
    using RangeIndexList = QList<RangeIndex>;

    explicit ViewGeometryHelper(FileView *parent = nullptr);

    RangeIndexList visibleIndexes(const QRect &rect) const;
    RangeIndexList rectContainsIndexes(const QRect &rect) const;
    RangeIndexList calcRectContiansIndexes(int columnCount, const QRect &rect) const;
    RangeIndexList calcGroupRectContiansIndexes(const QRect &rect) const;
    QRect calcVisualRect(int widgetWidth, int index) const;
    bool indexInRect(const QRect &actualRect, const QModelIndex &index) const;

private:
    // Pure calculation: column count from widget width and item width.
    // Depends only on view->itemSizeHint() and view->spacing() (public APIs).
    int calcColumnCount(int widgetWidth, int itemWidth = 0) const;

    FileView *m_view { nullptr };
};

}   // namespace dfmplugin_workspace

#endif   // VIEWGEOMETRYHELPER_H
