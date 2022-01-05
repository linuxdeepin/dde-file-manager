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
#ifndef CANVASGRIDMANAGER_H
#define CANVASGRIDMANAGER_H

#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/file/local/localfileinfo.h"
#include "dfm_desktop_service_global.h"

DSB_D_BEGIN_NAMESPACE

class CanvasGridManagerPrivate;
class CanvasGridManager : public QObject
{
    Q_OBJECT
public:
    static CanvasGridManager *instance();

public:
    void initCoord(const int screenCount);
    void initGridItemsInfo();
    void initArrage(const QList<DFMLocalFileInfoPointer> &items);
    void updateGridSize(const int screenNum, const int width, const int height);
    DFMLocalFileInfoPointer atPosFile(const int screenNum, const QPoint &pos);
    QPoint filePos(const int screenNum, const DFMLocalFileInfoPointer &info);
    QPoint filePos(const int screenNum, const QModelIndex &info);
    int emptyPostionCount(int screenNum) const;
    bool find(const QString &itemId, QPair<int, QPoint> &pos);
    QPoint pos5ition(int screenNum, const DFMLocalFileInfoPointer &file);
    bool isEmpty(int screenNum, int x, int y);
    bool contains(int screebNum, const QString &id);
    DFMLocalFileInfoPointer firstItemId(int screenNum);
    DFMLocalFileInfoPointer lastItemId(int screenNum);
    DFMLocalFileInfoPointer itemId(int screenNum, QPoint pos);
    DFMLocalFileInfoPointer itemTop(int screenNum, int x, int y);
    DFMLocalFileInfoPointer itemTop(int screenNum, QPoint pos);
    QList<DFMLocalFileInfoPointer> overlapItems() const;
    int overlapScreen();
    void allItems(QList<DFMLocalFileInfoPointer> &list) const;
    QHash<QPoint, DFMLocalFileInfoPointer> items(const int screenNum) const;
    void setShowHiddenFiles(bool value) noexcept;
    bool getShowHiddenFiles() noexcept;
    void delaySyncAllProfile(int ms = 100);
    void toggleArrange();
    bool autoArrange() const;

protected:
    explicit CanvasGridManager(QObject *parent = nullptr);
    ~CanvasGridManager();

private:
    CanvasGridManagerPrivate *d;
};

DSB_D_END_NAMESPACE
#endif   // CANVASGRIDMANAGER_H
