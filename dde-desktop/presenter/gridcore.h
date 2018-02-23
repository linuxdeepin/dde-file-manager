/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#pragma once

#include <QMap>
#include <QVector>
#include <QString>
#include <QPoint>
#include <QDebug>

typedef QPoint      GPos;
typedef qint32      GIndex;


inline QString positionKey(QPoint pos)
{
    return QString("%1_%2").arg(pos.x()).arg(pos.y());
}

/*!
 * \brief qQPointLessThanKey sort by postition
 * \param key1
 * \param key2
 * \return
 */
inline bool qQPointLessThanKey(const QPoint &key1, const QPoint &key2)
{
    return (key1.x() < key2.x()) ||
           ((key1.x() ==  key2.x()) && (key1.y() < key2.y()));
}

template <> inline bool qMapLessThanKey(const QPoint &key1, const QPoint &key2)
{
    return qQPointLessThanKey(key1, key2);
}


// TODO: move all grid calc to GridCore
class GridCore
{
public:

    QStringList           overlapItems;
    QMap<GPos, QString>   gridItems;
    QMap<QString, GPos>   itemGrids;
    QVector<bool>         gridStatus;

    QString               positionProfile;

    int                   coordWidth;
    int                   coordHeight;

public:
    GridCore();

    inline void addItem(GIndex index, const QString &item)
    {
        Q_ASSERT(index < gridStatus.length());
        gridStatus[index] = true;
        auto pos = toPos(index);
        itemGrids.insert(item, pos);
        gridItems.insert(pos, item);
    }

    inline void removeItem(GPos pos)
    {
        auto index = toIndex(pos);
        auto item = gridItems.take(pos);
        itemGrids.remove(item);
        Q_ASSERT(index < gridStatus.length());
        gridStatus[index] = false;
    }

    inline void removeItem(GIndex index)
    {
        auto pos = toPos(index);
        auto item = gridItems.take(pos);
        itemGrids.remove(item);
        Q_ASSERT(index < gridStatus.length());
        gridStatus[index] = false;
    }

    inline void removeItem(const QString &item)
    {
        auto pos = itemGrids.take(item);
        auto index = toIndex(pos);
        gridItems.take(pos);
        Q_ASSERT(index < gridStatus.length());
        gridStatus[index] = false;

    }

    inline GIndex toIndex(const GPos &pos) const
    {
        return pos.x() * coordHeight + pos.y();
    }

    inline GPos toPos(GIndex index) const
    {
        auto x = index / coordHeight;
        auto y = index % coordHeight;
        return GPos(x, y);
    }

    inline GPos pos(const QString &item) const
    {
        return itemGrids.value(item);
    }

    GIndex findEmptyForward(GIndex index, int emptyCount)
    {
        if (0 == emptyCount) {
            return index;
        }

        for (auto i = index; i >= 0; --i) {
            if (!gridStatus[i]) {
                --emptyCount;
                if (0 == emptyCount) {
                    return i;
                }
            }
        }
        return 0;
    }

    QStringList reloacleForward(GIndex start, GIndex end)
    {
        QStringList items;
        for (auto i = start; i <= end; ++i) {
            auto pos = toPos(i);
            if (gridItems.contains(pos)) {
                items << gridItems.value(pos);
                gridItems.remove(pos);
            }
        }

        for (auto i = start; i < start + items.length(); ++i) {
            auto pos = toPos(i);
            auto item = items.value(i - start);
            gridItems.insert(pos, item);
            itemGrids.insert(item, pos);
        }
        return items;
    }

    GIndex findEmptyBackward(GIndex index, int emptyCount)
    {
        if (0 == emptyCount) {
            return index;
        }

        for (auto i = index; i < gridStatus.length(); ++i) {
            if (!gridStatus[i]) {
                --emptyCount;
                if (0 == emptyCount) {
                    return i;
                }
            }
        }
        return gridStatus.length() - 1;
    }

    // start < end
    QStringList reloacleBackward(GIndex start, GIndex end)
    {
        QStringList items;
        for (auto i = end; i >= start; --i) {
            auto pos = toPos(i);
            if (gridItems.contains(pos)) {
                items << gridItems.value(pos);
                gridItems.remove(pos);
            }
        }

        for (auto i = end; i > end - items.length(); --i) {
            auto pos = toPos(i);
            auto item = items.value(end - i);
            gridItems.insert(pos, item);
            itemGrids.insert(item, pos);
        }
        return items;
    }


    QStringList reloacle(GIndex index, int emptyBefore, int emptyAfter);
};

