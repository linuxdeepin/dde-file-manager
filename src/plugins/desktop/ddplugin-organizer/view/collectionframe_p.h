/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef COLLECTIONFRAME_P_H
#define COLLECTIONFRAME_P_H

#include "collectionframe.h"

#include <QVBoxLayout>

DDP_ORGANIZER_BEGIN_NAMESPACE

class CollectionFramePrivate
{
public:
    enum ResponseArea {
        UnKnowRect = -1,
        LeftTopRect,
        TopRect,
        RightTopRect,
        RightRect,
        RightBottomRect,
        BottomRect,
        LeftBottomRect,
        LeftRect,
        TitleBarRect
    };

    enum CollectionFrameState {
        UnKnowState = -1,
        StretchState,
        MoveState,
        NormalShowState
    };

    explicit CollectionFramePrivate(CollectionFrame *qq = nullptr);
    ~CollectionFramePrivate();

    void updateAdjustRect();
    void updateMoveRect();
    ResponseArea getCurrentResponseArea(const QPoint &pos) const;
    void updateCursorState(const ResponseArea &stretchPlace);
    void updateMouseTrackingState();
    void updateFrameGeometry();

    inline bool canMove();
    bool canAdjust();

private:
    int calcLeftX();
    int calcRightX();
    int calcTopY();
    int calcBottomY();

public:
    CollectionFrame *const q = nullptr;
    QWidget *widget = nullptr;
    QWidget *titleBarWidget = nullptr;

    QVBoxLayout *mainLayout = nullptr;
    QRect titleBarRect;
    int minWidth = 20;
    int minHeight = 20;
    QList<QRect> stretchRects;
    QPoint stretchEndPoint;
    QRect adjustBeforRect;
    ResponseArea responseArea = UnKnowRect;
    QPoint moveStartPoint;
    QList<ResponseArea> stretchArea;
    QList<ResponseArea> moveArea;
    CollectionFrameState frameState = NormalShowState;

    CollectionFrame::CollectionFrameFeatures frameFeatures = CollectionFrame::NoCollectionFrameFeatures;
    CollectionFrame::CollectionFrameAdjust adjustStyle = CollectionFrame::CollectionFrameAdjustUnLimited;
};

DDP_ORGANIZER_END_NAMESPACE

#endif // COLLECTIONFRAME_P_H
