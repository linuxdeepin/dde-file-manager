/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#ifndef DEFAULTCANVASGRID_P_H
#define DEFAULTCANVASGRID_P_H

#include "defaultcanvasgridmanager.h"

uint qHash(const QPoint &key, uint seed = 0);

DSB_D_BEGIN_NAMESPACE

class DefaultCanvasGridManagerPrivate : public QObject
{
    Q_OBJECT
    friend class DefaultCanvasGridManager;

public:
    explicit DefaultCanvasGridManagerPrivate();
    DefaultCanvasGridManagerPrivate(DefaultCanvasGridManagerPrivate &) = delete;
    DefaultCanvasGridManagerPrivate &operator=(DefaultCanvasGridManagerPrivate &) = delete;

    ~DefaultCanvasGridManagerPrivate();

public:
    void loadProfile(const QList<DFMDesktopFileInfoPointer> &orderedItems, QHash<DFMDesktopFileInfoPointer, bool> existItems);
    void syncAllProfile();
    void syncProfile(const int screenNum);
    void autoArrange(QList<DFMDesktopFileInfoPointer> sortedItems);
    void clear();
    bool add(const int screenNum, const QPoint &pos, const DFMDesktopFileInfoPointer &info);
    bool add(const int screenNum, const DFMDesktopFileInfoPointer &info);
    bool add(const DFMDesktopFileInfoPointer &info);

    bool remove(const int screenNum, const DFMDesktopFileInfoPointer &info);
    bool remove(const DFMDesktopFileInfoPointer &info);

    void updateGridSize(const int screenNum, const int width, const int height);

    /*!
         * \brief 设置隐藏文件显示与否
         * \param \a value 隐藏文件显示与否，true则显示，否则不显示
         */
    inline void setWhetherShowHiddenFiles(bool value) noexcept
    {
        showHiddenFiles.store(value, std::memory_order_release);
    }

    /*!
         * \brief 获取显示隐藏文件
         * \return 是否显示隐藏文件,true则显示，反之则不显示
         */
    inline bool getWhetherShowHiddenFiles() noexcept
    {
        return showHiddenFiles.load(std::memory_order_consume);
    }

    /*!
         * \brief 获取实际屏幕编号
         * \return 返回所有屏幕编号
         */
    inline QList<int> screenCode() const
    {
        QList<int> screenOrder = screensCoordInfo.keys();
        qSort(screenOrder.begin(), screenOrder.end());
        return screenOrder;
    }

    /*!
         * \brief 获取堆叠图标位置
         * \return 返回堆叠图标位置
         */
    inline QPoint overlapPos()
    {
        auto coordInfo = screensCoordInfo.last();
        return QPoint(coordInfo.first - 1, coordInfo.second - 1);
    }

    /*!
         * \brief 获取指定屏幕和序号的坐标（行列）
         * \param \a screenNum 指定屏幕编号
         * \param \a index 指定屏幕中的指定序号
         * \return 指定屏幕下的指定序号对应的坐标(行列)
         */
    inline QPoint gridPosAt(const int screenNum, const int index)
    {
        auto coordHeight = screensCoordInfo.value(screenNum).second;
        auto x = index / coordHeight;
        auto y = index % coordHeight;
        return QPoint(x, y);
    }

    /*!
         * \brief 获取指定屏幕和坐标的序号
         * \param \a screenNum 指定屏幕编号
         * \param \a pos 指定屏幕中的指定位置
         * \return
         */
    inline int indexOfGridPos(const int screenNum, const QPoint &pos)
    {
        auto screenCoord = screensCoordInfo.value(screenNum);
        if (0 == screenCoord.first) {
            return 0;
        } else {
            return pos.x() * screenCoord.second + pos.y();
        }
    }

    /*!
         * \brief 判断屏幕指定位置是否合法
         * \param \a screenNum 指定屏幕编号
         * \param \a pos 指定屏幕中的位置
         * \return 返回是否合法
         */
    inline bool isValid(int screenNum, QPoint pos) const
    {
        auto coordInfo = screensCoordInfo.value(screenNum);
        return coordInfo.first > pos.x() && coordInfo.second > pos.y()
                && pos.x() >= 0 && pos.y() >= 0;
    }

    /*!
         * \brief 获取屏幕网格数量
         * \param \a screenNum 对应屏幕编号
         * \return 对应屏幕网格数量
         */
    inline int cellCount(int screenNum) const
    {
        auto coordInfo = screensCoordInfo.value(screenNum);
        return coordInfo.second * coordInfo.first;
    }

    void clearCoord();
    void clearPositionData();

private:
    QPair<QStringList, QVariantList> generateProfileVariable(int screenNum);
    void readProfileGroup();
    void writeProfileGroup();
    QPair<int, QPoint> takeEmptyPos(const int screenNum = 1);
    bool setCellStatus(const int screenNum, const int index, bool state);

public:
    // 屏幕配置文件 1=Screen_1     2=Screen_2
    QHash<int, QString> screenProfiles;
    // 屏幕编号-栅格行列数信息
    QMap<int, QPair<int, int>> screensCoordInfo;
    // 屏幕编号-是否有图标
    QHash<int, QVector<bool>> cellStatus;
    // 屏幕编号<栅格位置,文件info>
    QHash<int, QHash<QPoint, DFMDesktopFileInfoPointer>> gridItems;
    // 屏幕编号-<文件info,栅格位置>
    QHash<int, QHash<DFMDesktopFileInfoPointer, QPoint>> itemGrids;
    // 堆叠文件
    QList<DFMDesktopFileInfoPointer> overlapItems;
    // 所有文件
    QList<DFMDesktopFileInfoPointer> allItems;
    DefaultCanvasGridManager *const q { nullptr };

    bool singleScreen = false;
    bool isAutoArrange = false;
    std::atomic<bool> showHiddenFiles { false };
    bool addIntoOverlap = false;
};

DSB_D_END_NAMESPACE
#endif   // DEFAULTCANVASGRID_P_H
