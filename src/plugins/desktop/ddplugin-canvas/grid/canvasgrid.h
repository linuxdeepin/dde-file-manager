// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASGRID_H
#define CANVASGRID_H

#include "ddplugin_canvas_global.h"

#include <QObject>

extern uint qHash(const QPoint &key, uint seed);

namespace ddplugin_canvas {

class GridCore;
class CanvasGridPrivate;
class CanvasGrid : public QObject
{
    Q_OBJECT
    friend class CanvasGridPrivate;
public:
    enum class Mode {Custom = 0, Align};
public:
    static CanvasGrid *instance();
    void initSurface(int count = 0);

    void updateSize(int index, const QSize &size);
    QSize surfaceSize(int index) const;
    int gridCount(int index = -1) const;

    void setMode(Mode mode);
    Mode mode() const;

    void setItems(const QStringList &items);
    QStringList items(int index = -1) const;
    QString item(int index, const QPoint &pos) const;
    QHash<QString, QPoint> points(int index) const;
    bool point(const QString &item, QPair<int, QPoint> &pos) const;

    QStringList overloadItems(int index) const;

    bool drop(int index, const QPoint &pos, const QString &item);
    bool move(int toIndex, const QPoint &toPos, const QString &focus, const QStringList &items);
    bool remove(int index, const QString &item);
    bool replace(const QString &oldItem, const QString &newItem);
    void append(const QString &item);
    void append(const QStringList &items);
    void tryAppendAfter(const QStringList &items, int index, const QPoint &begin);
    void popOverload();
    void arrange();

    GridCore &core() const;
    void requestSync(int ms = 100);

protected:
    explicit CanvasGrid(QObject *parent = nullptr);
    ~CanvasGrid();
private:
    CanvasGridPrivate *d;
};

#define GridIns DDP_CANVAS_NAMESPACE::CanvasGrid::instance()
}

#endif // CANVASGRID_H
