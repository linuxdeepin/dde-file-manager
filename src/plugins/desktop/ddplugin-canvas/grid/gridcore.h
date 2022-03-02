/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef GRIDCORE_H
#define GRIDCORE_H

#include "canvasgridspecialist.h"

#include <QMap>
#include <QSize>

extern uint qHash(const QPoint &key, uint seed);

DDP_CANVAS_BEGIN_NAMESPACE

typedef QPair<int, QPoint> GridPos;
class GridCore
{
protected:
    explicit GridCore();
    explicit GridCore(const GridCore &other);

public:
    QList<int> surfaceIndex() const;
    virtual bool applay(GridCore *);
    virtual void insert(int index, const QPoint &pos, const QString &item);
    virtual void remove(int index, const QString &item);
    virtual void remove(int index, const QPoint &pos);
    virtual QList<QPoint> voidPos(int index) const;
    virtual bool findVoidPos(GridPos &pos) const;
    virtual bool isFull(int index) const;
    virtual bool position(const QString &item, GridPos &pos) const;
    virtual QString item(const GridPos &pos) const;
    virtual void removeAll(const QStringList &items);
public:
    inline QSize surfaceSize(int index) const {
        return surfaces.value(index, QSize(0, 0));
    }

    inline bool isValid(int index, const QPoint &pos) const {
        return CanvasGridSpecialist::isValid(pos, surfaceSize(index));
    }

    inline bool isVoid(int index, const QPoint &pos) {
        return !posItem.value(index).contains(pos);
    }

    inline void pushOverload(const QStringList &items){
        overload.append(items);
    }
public:
    QMap<int, QSize> surfaces;
    QMap<int, QHash<QPoint, QString>> posItem;
    QMap<int, QHash<QString, QPoint>> itemPos;
    QStringList overload;
};

class MoveGridOper : public GridCore
{
public:
    explicit MoveGridOper(GridCore *);
    bool move(const GridPos &to, const GridPos &center, const QStringList &moveItems);
    void calcDestination(const QStringList &orgItems, const GridPos &ref, const QPoint &focus,
                         QHash<QString, QPoint> &dest, QStringList &invalid);
};

class AppendOper : public GridCore
{
public:
    explicit AppendOper(GridCore *);
    void tryAppendAfter(QStringList items, int index, const QPoint &begin);
    void append(QStringList items);
    QStringList appendAfter(QStringList items, int index, const QPoint &begin);
};

DDP_CANVAS_END_NAMESPACE

#endif // GRIDCORE_H
