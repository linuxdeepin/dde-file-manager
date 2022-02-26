/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             liqiang<liqianga@uniontech.com>
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
#ifndef CANVASVIEW_P_H
#define CANVASVIEW_P_H

#include "canvasview.h"
#include "watermask/watermaskframe.h"
#include "model/canvasmodel.h"
#include "model/canvasselectionmodel.h"
#include "operator/clickselecter.h"
#include "operator/keyselecter.h"
#include "operator/operstate.h"
#include "operator/dragdropoper.h"
#include "operator/shortcutoper.h"
#include "operator/canvasviewmenuproxy.h"
#include "operator/viewsettingutil.h"
#include "gridcoordinate.h"
#include "dfm_desktop_service_global.h"

#include <QDebug>

DSB_D_BEGIN_NAMESPACE

class CanvasViewPrivate : public QObject
{
    Q_OBJECT
public:
    struct CanvasInfo
    {
        CanvasInfo(){}
        CanvasInfo(int column, int row, int width, int height)
            : rowCount(row)
            , columnCount(column)
            , gridWidth(width)
            , gridHeight(height)
        {

        }

        inline int gridCount() const {
            return columnCount * rowCount;
        }

        int rowCount = 1;
        int columnCount = 1;
        int gridWidth = 1;
        int gridHeight = 1;
    };
public:
    explicit CanvasViewPrivate(CanvasView *qq);
    ~CanvasViewPrivate();
    void updateGridSize(const QSize &viewSize, const QMargins &geometryMargins, const QSize &itemSize);
    QMargins calcMargins(const QSize &inSize, const QSize &outSize);
    QRect visualRect(const QPoint &gridPos) const;
    QRect visualRect(const QString &item) const;
    QString visualItem(const QPoint &gridPos) const;
    bool isEmptyArea(const QPoint &pos) const;
    bool isWaterMaskOn();
public:
    QModelIndex findIndex(const QString &key, bool matchStart, const QModelIndex &current, bool reverseOrder, bool excludeCurrent) const;
    QModelIndex firstIndex() const;
    QModelIndex lastIndex() const;
public:
    inline GridCoordinate gridCoordinate(int index) const {
        return GridCoordinate(index / canvasInfo.rowCount, index % canvasInfo.rowCount);
    }

    inline int gridIndex(const QPoint &gridPos) const{
        return gridPos.x() * canvasInfo.rowCount + gridPos.y();
    }

    inline QPoint overlapPos() const {
        return QPoint(canvasInfo.columnCount - 1, canvasInfo.rowCount - 1);
    }

    inline QPoint gridAt(const QPoint &viewPos) const {
        auto row = (viewPos.x() - viewMargins.left()) / canvasInfo.gridWidth;
        auto col = (viewPos.y() - viewMargins.top()) / canvasInfo.gridHeight;
        return QPoint(row, col);
    }

    inline QRect itemRect(const QString &item) const {
        return visualRect(item).marginsRemoved(gridMargins);
    }

    QRect itemRect(const QPoint &gridPos) const {
        return visualRect(gridPos).marginsRemoved(gridMargins);
    }

    inline OperState &operState() {
        return state;
    }
public: // 绘制扩展的特殊处理
    static inline QMargins gridMarginsHelper(CanvasView *view){
        QMargins margins(0, 0, 0, 0);
        if (view)
            margins = view->d->gridMargins;
        return margins;
    }
public:
    static const QMargins gridMiniMargin;
    static const QSize dockReserveSize;
    bool showGrid = false;
    int screenNum;

    CanvasInfo canvasInfo;
    QMargins gridMargins;  // grid inner margin.
    QMargins viewMargins;  // view margin is to decrease canvas rect on view.

    ClickSelecter *clickSelecter;
    KeySelecter *keySelecter;
    DragDropOper *dragDropOper;
    ShortcutOper *shortcutOper;
    ViewSettingUtil *viewSetting;
    OperState state;
    bool flicker = false;

    QPoint dragTargetGrid { QPoint(-1, -1) };

    CanvasViewMenuProxy *menuProxy = nullptr;
    WaterMaskFrame *waterMask = nullptr;
    CanvasView *q;
};

DSB_D_END_NAMESPACE

#endif   // CANVASVIEW_P_H
