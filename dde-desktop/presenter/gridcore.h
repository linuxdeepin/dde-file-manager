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

#if 0
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

#endif
#if 1
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

    QMap<int, QMap<GPos, QString>> gridItems;
    QMap<int, QMap<QString, GPos>> itemGrids;
    QMap<int, bool>             m_screenFullStatus;//屏幕图标状态
    QMap<int, QVector<bool>>    m_cellStatus;//<screenNum, QVector<bool>>
    QMap<int,QString>           positionProfiles;
    QMap<int,QPair<int, int>>   screensCoordInfo;//<screenNum,<coordWidth,coordHeight>>

public:
    GridCore();

    void addItem(int screenNum, GIndex index, const QString &item)
    {
        Q_ASSERT(index < m_cellStatus.value(screenNum).length());
        if (!m_cellStatus.contains(screenNum)) {
            qDebug()<< "can not find num :" << screenNum;
            return;
        }
        auto cellStatusItor = m_cellStatus.find(screenNum);
        cellStatusItor.value()[index] = true;
        auto pos = toPos(screenNum, index);

        if (!itemGrids.contains(screenNum)
                ||!gridItems.contains(screenNum)) {
            qDebug()<< "can not find num :" << screenNum;
            return;
        }
        auto tempGridItemsItor = gridItems.find(screenNum);
        auto tempItemGridsItor = itemGrids.find(screenNum);
        tempItemGridsItor->insert(item,pos);
        tempGridItemsItor->insert(pos,item);
    }

    void removeItem(int screenNum, GPos pos)
    {
        if(!gridItems.contains(screenNum) || !itemGrids.contains(screenNum) ){
            qDebug()<< "can not find num :" << screenNum;
            return;
        }
        auto tempGridItemsItor = gridItems.find(screenNum);
        auto tempItemGridsItor = itemGrids.find(screenNum);

        auto index = toIndex(screenNum, pos);
        auto item = tempGridItemsItor.value().take(pos);
        tempItemGridsItor.value().remove(item);

        if (!m_cellStatus.contains(screenNum)) {
            qDebug()<< "can not find num :" << screenNum;
            return;
        }
        Q_ASSERT(index < m_cellStatus.value(screenNum).length());
         auto cellStatusItor = m_cellStatus.find(screenNum);
        cellStatusItor.value()[index] = false;
    }

    void removeItem(int screenNum, GIndex index)
    {
        if(!gridItems.contains(screenNum) || !itemGrids.contains(screenNum) ){
            qDebug()<< "can not find num :" << screenNum;
            return;
        }
        auto tempGridItemsItor = gridItems.find(screenNum);
        auto tempItemGridsItor = itemGrids.find(screenNum);


        auto pos = toPos(screenNum, index);
        auto item = tempGridItemsItor.value().take(pos);
        tempItemGridsItor.value().remove(item);


        if (!m_cellStatus.contains(screenNum)) {
            qDebug()<< "can not find num :" << screenNum;
            return;
        }
        Q_ASSERT(index < m_cellStatus.value(screenNum).length());
         auto cellStatusItor = m_cellStatus.find(screenNum);
        cellStatusItor.value()[index] = false;
    }

    void removeItem(int screenNum, const QString &item)
    {
        if(!gridItems.contains(screenNum) || !itemGrids.contains(screenNum) ){
            qDebug()<< "can not find num :" << screenNum;
            return;
        }
        auto tempGridItemsItor = gridItems.find(screenNum);
        auto tempItemGridsItor = itemGrids.find(screenNum);

        auto pos = tempItemGridsItor.value().take(item);
        auto index = toIndex(screenNum, pos);
        tempGridItemsItor.value().take(pos);

        if (!m_cellStatus.contains(screenNum)) {
            qDebug()<< "can not find num :" << screenNum;
            return;
        }
        Q_ASSERT(index < m_cellStatus.value(screenNum).length());
         auto cellStatusItor = m_cellStatus.find(screenNum);
        cellStatusItor.value()[index] = false;

    }

    inline GIndex toIndex(int screenNum, const GPos &pos) const
    {
        auto coordInfo = screensCoordInfo.value(screenNum);
        return pos.x() * coordInfo.second + pos.y();
    }

    inline GPos toPos(int screenNum, GIndex index) const
    {
        auto coordInfo = screensCoordInfo.value(screenNum);

        auto x = index / coordInfo.second;
        auto y = index % coordInfo.second;
        return GPos(x, y);
    }

    inline GPos pos(int screenNum, const QString &item) const
    {
        return itemGrids.value(screenNum).value(item);
    }

    GIndex findEmptyForward(int screenNum, GIndex index, int emptyCount)
    {
        if (0 == emptyCount) {
            return index;
        }
        if (!m_cellStatus.contains(screenNum)) {
            qDebug()<< "can not find num :" << screenNum;
            return index;//return right?
        }
        auto cellStatusItor = m_cellStatus.find(screenNum);

        for (auto i = index; i >= 0; --i) {
            if (!cellStatusItor.value()[i]) {
                --emptyCount;
                if (0 == emptyCount) {
                    return i;
                }
            }
        }
        return 0;
    }

    QStringList reloacleForward(int screenNum, GIndex start, GIndex end)
    {
        QStringList items;
        if(!gridItems.contains(screenNum) || !itemGrids.contains(screenNum) ){
            qDebug()<< "can not find num :" << screenNum;
            return items;
        }
        auto tempGridItemsItor = gridItems.find(screenNum);
        auto tempItemGridsItor = itemGrids.find(screenNum);

        for (auto i = start; i <= end; ++i) {
            auto pos = toPos(screenNum, i);
            if (tempGridItemsItor.value().contains(pos)) {
                items << tempGridItemsItor.value().value(pos);
                tempGridItemsItor.value().remove(pos);
            }
        }

        for (auto i = start; i < start + items.length(); ++i) {
            auto pos = toPos(screenNum,i);
            auto item = items.value(i - start);
            tempGridItemsItor.value().insert(pos, item);
            tempItemGridsItor.value().insert(item, pos);
        }
        return items;
    }

    GIndex findEmptyBackward(int screenNum, GIndex index, int emptyCount)
    {
        if (!m_cellStatus.contains(screenNum)) {
            qDebug()<< "can not find num :" << screenNum;
            return index;//return right?
        }
        auto cellStatusItor = m_cellStatus.find(screenNum);

        if (0 == emptyCount) {
            return index;
        }

        for (auto i = index; i < m_cellStatus.value(screenNum).length(); ++i) {
            if (!cellStatusItor.value()[i]) {
                --emptyCount;
                if (0 == emptyCount) {
                    return i;
                }
            }
        }
        return m_cellStatus.value(screenNum).length() - 1;
    }

    // start < end
    QStringList reloacleBackward(int screenNum, GIndex start, GIndex end)
    {
        QStringList items;
        if(!gridItems.contains(screenNum) || !itemGrids.contains(screenNum) ){
            qDebug()<< "can not find num :" << screenNum;
            return items;
        }
        auto tempGridItemsItor = gridItems.find(screenNum);
        auto tempItemGridsItor = itemGrids.find(screenNum);

        for (auto i = end; i >= start; --i) {
            auto pos = toPos(screenNum, i);
            if (tempGridItemsItor.value().contains(pos)) {
                items << tempGridItemsItor.value().value(pos);
                tempGridItemsItor.value().remove(pos);
            }
        }

        for (auto i = end; i > end - items.length(); --i) {
            auto pos = toPos(screenNum, i);
            auto item = items.value(end - i);
            tempGridItemsItor.value().insert(pos, item);
            tempItemGridsItor.value().insert(item, pos);
        }
        return items;
    }

    QList<GIndex> emptyPostion(int screenNum) const {
        QList<GIndex> ret;
        if (!m_cellStatus.contains(screenNum)) {
            qDebug() << "can not find num :" << screenNum;
            return ret;
        }

        auto cellStatusItor = m_cellStatus.find(screenNum);
        for (GIndex i = 0; i < m_cellStatus.value(screenNum).length(); ++i) {
            if (!cellStatusItor.value()[i]) {
                ret.append(i);
            }
        }
        return ret;
    }

    QStringList reloacle(int screenNum, GIndex index, int emptyBefore, int emptyAfter);
};
#endif

