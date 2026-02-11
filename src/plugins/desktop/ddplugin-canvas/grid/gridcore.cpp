// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gridcore.h"
#include "displayconfig.h"

#include <QHashFunctions>

uint qHash(const QPoint &key, uint seed)
{
    return qHash(qMakePair(key.x(), key.y()), seed);
}

using namespace ddplugin_canvas;

GridCore::GridCore()
{
}

GridCore::GridCore(const GridCore &other)
    : surfaces(other.surfaces), posItem(other.posItem), itemPos(other.itemPos), overload(other.overload)
{
}

GridCore::~GridCore()
{
}

QList<int> GridCore::surfaceIndex() const
{
    QList<int> order = surfaces.keys();
    std::stable_sort(order.begin(), order.end(), [](const int &first, const int &second) {
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

void GridCore::insert(int index, const QPoint &pos, const QString &it)
{
    itemPos[index].insert(it, pos);
    posItem[index].insert(pos, it);
}

void GridCore::remove(int index, const QString &it)
{
    auto pos = itemPos[index].take(it);
    posItem[index].remove(pos);
}

void GridCore::remove(int index, const QPoint &pos)
{
    QString it = posItem[index].take(pos);
    itemPos[index].remove(it);
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
            continue;

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

bool GridCore::position(const QString &it, GridPos &pos) const
{
    bool find = false;
    for (auto itor = itemPos.begin(); itor != itemPos.end(); ++itor) {
        if (itor.value().contains(it)) {
            find = true;
            pos.first = itor.key();
            pos.second = itor.value().value(it);
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
        for (const QString &it : items) {

            overload.removeAll(it);

            if (!itemPos[index].contains(it))
                continue;
            auto pos = itemPos[index].take(it);
            posItem[index].remove(pos);
        }
    }
}

MoveGridOper::MoveGridOper(GridCore *core)
    : GridCore(*core)
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
    for (const QString &it : orgItems) {

        if (Q_UNLIKELY(it.isEmpty()))
            continue;

        if (position(it, tempPos)) {
            // not from one surface
            if (tempPos.first != ref.first) {
                invalid.append(it);
            } else {
                auto target = tempPos.second - ref.second + focus;
                if (isValid(ref.first, target))
                    dest.insert(it, target);
                else   // invalid pos
                    invalid.append(it);
            }
        } else {   // origin pos is invalid.
            invalid.append(it);
        }
    }
}

AppendOper::AppendOper(GridCore *core)
    : GridCore(*core)
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
        if ((pos.x() > begin.x())
            || (pos.x() == begin.x() && pos.y() >= begin.y())
            || DisplayConfig::instance()->autoAlign()) {
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

            QString &&it = items.takeFirst();
            insert(idx, pos, it);
        }
    }

    // overload
    if (!items.isEmpty())
        pushOverload(items);
}
