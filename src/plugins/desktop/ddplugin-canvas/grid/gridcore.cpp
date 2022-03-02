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
#include "gridcore.h"

uint qHash(const QPoint &key, uint seed)
{
    QString val = QString("%1x%2").arg(key.x()).arg(key.y());
    return qHash<QString>(val, seed);
}

DDP_CANVAS_USE_NAMESPACE

GridCore::GridCore()
{

}

GridCore::GridCore(const GridCore &other)
    : surfaces(other.surfaces)
    , posItem(other.posItem)
    , itemPos(other.itemPos)
    , overload(other.overload)
{
}

QList<int> GridCore::surfaceIndex() const
{
    QList<int> order = surfaces.keys();
    qSort(order.begin(), order.end(), [](const int &first, const int &second){
        return first < second;
    });
    return order;
}

bool GridCore::applay(GridCore *core)
{
    if (!core)
        return false;

    surfaces = core->surfaces;
    posItem = core->posItem;
    itemPos = core->itemPos;
    overload = core->overload;
    return true;
}

void GridCore::insert(int index, const QPoint &pos, const QString &item)
{
    itemPos[index].insert(item, pos);
    posItem[index].insert(pos, item);
}

void GridCore::remove(int index, const QString &item)
{
    auto pos = itemPos[index].take(item);
    posItem[index].remove(pos);
}

void GridCore::remove(int index, const QPoint &pos)
{
    auto item = posItem[index].take(pos);
    itemPos[index].remove(item);
}

QList<QPoint> GridCore::voidPos(int index) const
{
    QList<QPoint> ret;
    const QSize &size = surfaces.value(index, QSize(0, 0));
    const QHash<QPoint, QString> &usedPos = posItem.value(index);
    for (int x = 0; x < size.width(); ++x)
        for (int y = 0; y < size.height(); ++y) {
            QPoint pos(x, y);
            if (!usedPos.contains(pos))
                ret.append(pos);
        }

    return ret;
}

bool GridCore::findVoidPos(GridPos &pos) const
{
    for (int idx : surfaceIndex()) {
        const QHash<QPoint, QString> &usedPos = posItem.value(idx);
        const QSize &size = surfaces.value(idx);

        // no void pos
        if (isFull(idx))
            break;

        // find first void pos.
        for (int x = 0; x < size.width(); ++x)
            for (int y = 0; y < size.height(); ++y) {
                QPoint curPos(x, y);
                if (!usedPos.contains(curPos)) {
                    pos.first = idx;
                    pos.second = curPos;
                    return true;
                }
            }
    }

    return false;
}

bool GridCore::isFull(int index) const
{
    const QSize &size = surfaces.value(index, QSize(0, 0));
    int itemCount = posItem.value(index).count();

    return itemCount >= size.width() * size.height();
}

bool GridCore::position(const QString &item, GridPos &pos) const
{
    bool find = false;
    for (auto itor = itemPos.begin(); itor != itemPos.end(); ++itor) {
        if (itor.value().contains(item)) {
            find = true;
            pos.first = itor.key();
            pos.second = itor.value().value(item);
            break;
        }
    }

    return find;
}

QString GridCore::item(const GridPos &pos) const
{
    return posItem.value(pos.first).value(pos.second);
}

void GridCore::removeAll(const QStringList &items)
{
    for (int index : itemPos.keys()) {
        for (const QString &item : items) {

            overload.removeAll(item);

            if (!itemPos[index].contains(item))
                continue;
            auto pos = itemPos[index].take(item);
            posItem[index].remove(pos);
        }
    }
}


MoveGridOper::MoveGridOper(GridCore *core) : GridCore(*core)
{

}

bool MoveGridOper::move(const GridPos &to, const GridPos &center, const QStringList &moveItems)
{
    QHash<QString, QPoint> destPos;
    QStringList invalidPos;

    // calculate target pos of \a moveItems
    calcDestination(moveItems, center, to.second, destPos, invalidPos);

    // pick up all items which will be moved to let it's postion to be void.
    removeAll(moveItems);

    {
        QStringList unDrop;
        for (auto itor = destPos.begin(); itor != destPos.end(); ++itor) {
            // if target pos is void, drop the item.
            if (isVoid(to.first, itor.value()))
                insert(to.first, itor.value(), itor.key());
            else
                unDrop.append(itor.key());
        }

        // item failed to drop need apeend before invalid one.
        invalidPos = unDrop + invalidPos;
    }

    // append all items that can not move to \a destPos
    if (!invalidPos.isEmpty()) {
        AppendOper oper(this);
        oper.tryAppendAfter(invalidPos, to.first, to.second);
        applay(&oper);
    }

    return true;
}

void MoveGridOper::calcDestination(const QStringList &orgItems, const GridPos &ref, const QPoint &focus,
                                   QHash<QString, QPoint> &dest, QStringList &invalid)
{
    GridPos tempPos;
    for (const QString &item : orgItems) {

        if (Q_UNLIKELY(item.isEmpty()))
            continue;

        if (position(item, tempPos)) {
            // not from one surface
            if (tempPos.first != ref.first) {
                invalid.append(item);
            } else {
                auto target = tempPos.second - ref.second + focus;
                if (isValid(ref.first, target))
                    dest.insert(item, target);
                else // invalid pos
                    invalid.append(item);
            }
        } else { // origin pos is invalid.
            invalid.append(item);
        }
    }
}

AppendOper::AppendOper(GridCore *core) : GridCore(*core)
{

}

void AppendOper::tryAppendAfter(QStringList items, int index, const QPoint &begin)
{
    items = appendAfter(items, index, begin);

    // left over
    if (items.isEmpty())
        return;

    // apeend begin head on the surface
    const QPoint surfaceHead(0, 0);
    if (begin != surfaceHead)
        items = appendAfter(items, index, surfaceHead);

    // still left over
    if (items.isEmpty())
        return;

    // apeend begin first surace
    append(items);
}

QStringList AppendOper::appendAfter(QStringList items, int index, const QPoint &begin)
{
    if (items.isEmpty())
        return items;

    QList<QPoint> posList = voidPos(index);
    for (const QPoint &pos : posList) {
        if ((pos.x() > begin.x()) || (pos.x() == begin.x() && pos.y() >= begin.y())) {
            // all items is appenped
            if (items.isEmpty())
                return items;

            QString &&item = items.takeFirst();
            insert(index, pos, item);
        }
    }

    return items;
}

void AppendOper::append(QStringList items)
{
    for (int idx : surfaceIndex()) {
        auto posList = voidPos(idx);
        for (const QPoint &pos : posList) {
            // all items is appenped
            if (items.isEmpty())
                return;

            QString &&item = items.takeFirst();
            insert(idx, pos, item);
        }
    }

    // overload
    if (!items.isEmpty())
        pushOverload(items);
}
