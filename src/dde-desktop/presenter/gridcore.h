// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

    QMap<int, QMap<GPos, QString>> gridItems;
    QMap<int, QMap<QString, GPos>> itemGrids;
    QMap<int, bool>             m_screenFullStatus;//屏幕图标状态
    QMap<int, QVector<bool>>    m_cellStatus;//<screenNum, QVector<bool>>
    QMap<int, QString>           positionProfiles;
    QMap<int, QPair<int, int>>   screensCoordInfo; //<screenNum,<coordWidth,coordHeight>>

public:
    GridCore();

    void addItem(int screenNum, GIndex index, const QString &item)
    {
        //bug#45219，当出现错误的index时，放入堆叠
        if (index < 0 || index >= m_cellStatus.value(screenNum).length()) {
            qWarning() << "screen" << screenNum << "error index" << index << item;
            if (!overlapItems.contains(item)) {
                overlapItems << item;
            }
            return;
        }

        if (!m_cellStatus.contains(screenNum)) {
            qDebug() << "can not find num :" << screenNum;
            return;
        }
        auto cellStatusItor = m_cellStatus.find(screenNum);
        cellStatusItor.value()[index] = true;
        auto position = toPos(screenNum, index);

        if (!itemGrids.contains(screenNum)
                || !gridItems.contains(screenNum)) {
            qDebug() << "can not find num :" << screenNum;
            return;
        }
        auto tempGridItemsItor = gridItems.find(screenNum);
        auto tempItemGridsItor = itemGrids.find(screenNum);
        tempItemGridsItor->insert(item, position);
        tempGridItemsItor->insert(position, item);
    }

    void removeItem(int screenNum, GPos pos)
    {
        if (!gridItems.contains(screenNum) || !itemGrids.contains(screenNum)) {
            qDebug() << "can not find num :" << screenNum;
            return;
        }
        auto tempGridItemsItor = gridItems.find(screenNum);
        auto tempItemGridsItor = itemGrids.find(screenNum);

        auto index = toIndex(screenNum, pos);
        auto item = tempGridItemsItor.value().take(pos);
        tempItemGridsItor.value().remove(item);

        if (!m_cellStatus.contains(screenNum)) {
            qDebug() << "can not find num :" << screenNum;
            return;
        }
        Q_ASSERT(index < m_cellStatus.value(screenNum).length());
        auto cellStatusItor = m_cellStatus.find(screenNum);
        cellStatusItor.value()[index] = false;
    }

    void removeItem(int screenNum, GIndex index)
    {
        if (!gridItems.contains(screenNum) || !itemGrids.contains(screenNum)) {
            qDebug() << "can not find num :" << screenNum;
            return;
        }
        auto tempGridItemsItor = gridItems.find(screenNum);
        auto tempItemGridsItor = itemGrids.find(screenNum);


        auto position = toPos(screenNum, index);
        auto item = tempGridItemsItor.value().take(position);
        tempItemGridsItor.value().remove(item);


        if (!m_cellStatus.contains(screenNum)) {
            qDebug() << "can not find num :" << screenNum;
            return;
        }
        Q_ASSERT(index < m_cellStatus.value(screenNum).length());
        auto cellStatusItor = m_cellStatus.find(screenNum);
        cellStatusItor.value()[index] = false;
    }

    void removeItem(int screenNum, const QString &item)
    {
        if (!gridItems.contains(screenNum) || !itemGrids.contains(screenNum)) {
            qDebug() << "can not find num :" << screenNum;
            return;
        }
        auto tempGridItemsItor = gridItems.find(screenNum);
        auto tempItemGridsItor = itemGrids.find(screenNum);

        auto position = tempItemGridsItor.value().take(item);
        auto index = toIndex(screenNum, position);
        tempGridItemsItor.value().take(position);

        if (!m_cellStatus.contains(screenNum)) {
            qDebug() << "can not find num :" << screenNum;
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
        if (itemGrids.value(screenNum).contains(item)) {
            return itemGrids.value(screenNum).value(item);
        } else {
            auto coordInfo = screensCoordInfo.value(screenNum);
            return GPos(coordInfo.first - 1, coordInfo.second - 1);
        }
    }

    GIndex findEmptyForward(int screenNum, GIndex index, int emptyCount)
    {
        if (0 == emptyCount) {
            return index;
        }
        if (!m_cellStatus.contains(screenNum)) {
            qDebug() << "can not find num :" << screenNum;
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
        if (!gridItems.contains(screenNum) || !itemGrids.contains(screenNum)) {
            qDebug() << "can not find num :" << screenNum;
            return items;
        }
        auto tempGridItemsItor = gridItems.find(screenNum);
        auto tempItemGridsItor = itemGrids.find(screenNum);

        for (auto i = start; i <= end; ++i) {
            auto position = toPos(screenNum, i);
            if (tempGridItemsItor.value().contains(position)) {
                items << tempGridItemsItor.value().value(position);
                tempGridItemsItor.value().remove(position);
            }
        }

        for (auto i = start; i < start + items.length(); ++i) {
            auto position = toPos(screenNum, i);
            auto item = items.value(i - start);
            tempGridItemsItor.value().insert(position, item);
            tempItemGridsItor.value().insert(item, position);
        }
        return items;
    }

    GIndex findEmptyBackward(int screenNum, GIndex index, int emptyCount)
    {
        if (!m_cellStatus.contains(screenNum)) {
            qDebug() << "can not find num :" << screenNum;
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
        if (!gridItems.contains(screenNum) || !itemGrids.contains(screenNum)) {
            qDebug() << "can not find num :" << screenNum;
            return items;
        }
        auto tempGridItemsItor = gridItems.find(screenNum);
        auto tempItemGridsItor = itemGrids.find(screenNum);

        for (auto i = end; i >= start; --i) {
            auto position = toPos(screenNum, i);
            if (tempGridItemsItor.value().contains(position)) {
                items << tempGridItemsItor.value().value(position);
                tempGridItemsItor.value().remove(position);
            }
        }

        for (auto i = end; i > end - items.length(); --i) {
            auto position = toPos(screenNum, i);
            auto item = items.value(end - i);
            tempGridItemsItor.value().insert(position, item);
            tempItemGridsItor.value().insert(item, position);
        }
        return items;
    }

    QList<GIndex> emptyPostion(int screenNum) const
    {
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

    QStringList reloacle(int screenNum, GIndex targetIndex, int emptyBefore, int emptyAfter);
};

