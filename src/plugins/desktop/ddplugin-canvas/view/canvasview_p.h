// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASVIEW_P_H
#define CANVASVIEW_P_H

#include "ddplugin_canvas_global.h"
#include "canvasview.h"
#include "watermask/watermaskcontainer.h"
#include "model/canvasproxymodel.h"
#include "model/canvasselectionmodel.h"
#include "operator/clickselector.h"
#include "operator/keyselector.h"
#include "operator/operstate.h"
#include "operator/dragdropoper.h"
#include "operator/dodgeoper.h"
#include "operator/sortanimationoper.h"
#include "operator/shortcutoper.h"
#include "operator/canvasviewmenuproxy.h"
#include "operator/viewsettingutil.h"
#include "gridcoordinate.h"
#include "viewhookinterface.h"

#include <QDebug>

namespace ddplugin_canvas {

class CanvasViewPrivate : public QObject
{
    Q_OBJECT
public:
    enum class ClickedAction : uint8_t {
        kClicked = 0,
        kDoubleClicked
    };

    struct CanvasInfo
    {
        CanvasInfo() { }
        CanvasInfo(int column, int row, int width, int height)
            : rowCount(row), columnCount(column), gridWidth(width), gridHeight(height)
        {
        }

        inline int gridCount() const
        {
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
    QString visualItem(const QPoint &gridPos) const;
    bool itemGridpos(const QString &item, QPoint &gridPos) const;
    bool isWaterMaskOn();
    void openIndexByClicked(const ClickedAction action, const QModelIndex &index);
    void openIndex(const QModelIndex &index);

public:
    QModelIndex findIndex(const QString &key, bool matchStart, const QModelIndex &current, bool reverseOrder, bool excludeCurrent) const;
    QModelIndex firstIndex() const;
    QModelIndex lastIndex() const;

public:
    inline GridCoordinate gridCoordinate(int index) const
    {
        return GridCoordinate(index / canvasInfo.rowCount, index % canvasInfo.rowCount);
    }

    inline int gridIndex(const QPoint &gridPos) const
    {
        return gridPos.x() * canvasInfo.rowCount + gridPos.y();
    }

    inline QPoint overlapPos() const
    {
        return QPoint(canvasInfo.columnCount - 1, canvasInfo.rowCount - 1);
    }

    inline QPoint gridAt(const QPoint &point) const
    {
        auto row = (point.x() - viewMargins.left()) / canvasInfo.gridWidth;
        auto col = (point.y() - viewMargins.top()) / canvasInfo.gridHeight;
        return QPoint(row, col);
    }

    QRect itemRect(const QPoint &gridPos) const
    {
        return visualRect(gridPos).marginsRemoved(gridMargins);
    }

    inline OperState &operState()
    {
        return state;
    }

public:   // 绘制扩展的特殊处理
    static inline QMargins gridMarginsHelper(CanvasView *view)
    {
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
    QMargins gridMargins;   // grid inner margin.
    QMargins viewMargins;   // view margin is to decrease canvas rect on view.

    ClickSelector *clickSelector;
    KeySelector *keySelector;
    DragDropOper *dragDropOper;
    DodgeOper *dodgeOper;
    SortAnimationOper *sortAnimOper;
    ShortcutOper *shortcutOper;
    ViewSettingUtil *viewSetting;
    OperState state;
    bool flicker = false;

    CanvasViewMenuProxy *menuProxy = nullptr;
    WatermaskContainer *waterMask = nullptr;
    ViewHookInterface *hookIfs = nullptr;
    CanvasView *q;
};

}

#endif   // CANVASVIEW_P_H
