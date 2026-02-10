// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLICKSELECTOR_H
#define CLICKSELECTOR_H

#include "ddplugin_canvas_global.h"

#include <QObject>
#include <QModelIndex>

namespace ddplugin_canvas {
class CanvasView;
class ClickSelector : public QObject
{
    Q_OBJECT
public:
    explicit ClickSelector(CanvasView *parent);
    void click(const QModelIndex &index);
    void release(const QModelIndex &index);
signals:

public slots:
protected:
    static void order(const QPoint &p1, const QPoint &p2, QPoint &from, QPoint &to);
    static QList<QPoint> horizontalTraversal(const QPoint &from, const QPoint &to, const QSize &size);
    void clear();
    void expandSelect(const QModelIndex &index);
    void continuesSelect(const QModelIndex &index);
    void traverseSelect(const QModelIndex &from, const QModelIndex &to);
    void traverseSelect(const QPoint &p1, const QPoint &p2);
private:
    void singleSelect(const QModelIndex &index);
protected:
    CanvasView *view;
    QPersistentModelIndex lastPressedIndex;
    QPersistentModelIndex toggleIndex;
};
}
#endif // CLICKSELECTOR_H
