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
#ifndef DEFAULTCANVASGRID_H
#define DEFAULTCANVASGRID_H

#include "dfm-base/widgets/abstractcanvasgridmanager.h"
#include "dfm-base/base/abstractfileinfo.h"
#include "defaultdesktopfileinfo.h"

DSB_D_BEGIN_NAMESPACE

class DefaultCanvasGridManagerPrivate;
class DefaultCanvasGridManager : public dfmbase::AbstractCanvasGridManager
{
public:
    static DefaultCanvasGridManager *instance();
    void initCoord(const int screenCount);
    void initGridItemsInfo();
    void initArrage(const QList<DFMDesktopFileInfoPointer> &items);
    void updateGridSize(const int screenNum, const int width, const int height);
    DFMDesktopFileInfoPointer atPosFile(const int screenNum, const QPoint &pos);
    QPoint filePos(const int screenNum, const DFMDesktopFileInfoPointer &info);
    QPoint filePos(const int screenNum, const QModelIndex &info);
    int emptyPostionCount(int screenNum) const;
    bool find(const QString &itemId, QPair<int, QPoint> &pos);
    QPoint pos5ition(int screenNum, const DFMDesktopFileInfoPointer &file);
    bool isEmpty(int screenNum, int x, int y);
    bool contains(int screebNum, const QString &id);
    DFMDesktopFileInfoPointer firstItemId(int screenNum);
    DFMDesktopFileInfoPointer lastItemId(int screenNum);
    DFMDesktopFileInfoPointer itemId(int screenNum, QPoint pos);
    DFMDesktopFileInfoPointer itemTop(int screenNum, int x, int y);
    DFMDesktopFileInfoPointer itemTop(int screenNum, QPoint pos);
    QList<DFMDesktopFileInfoPointer> overlapItems() const;
    int overLapScreen();
    void allItems(QList<DFMDesktopFileInfoPointer> &list) const;
    QHash<QPoint, DFMDesktopFileInfoPointer> items(const int screenNum) const;
    void setShowHiddenFiles(bool value) noexcept;
    bool getShowHiddenFiles() noexcept;
    void delaySyncAllProfile(int ms = 100);
    void toggleArrange();
    bool autoArrange() const;

private:

    DefaultCanvasGridManager(QObject *parent = nullptr);
    QScopedPointer<DefaultCanvasGridManagerPrivate> d;
};
DSB_D_END_NAMESPACE
#endif   // DEFAULTCANVASGRID_H
