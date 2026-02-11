// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLECTIONFRAME_P_H
#define COLLECTIONFRAME_P_H

#include "collectionframe.h"
#include "private/surface.h"

#include <QVBoxLayout>

namespace ddplugin_organizer {

class CollectionFramePrivate
{
public:
    enum ResponseArea {
        UnKnowRect = -1,
        TitleBarRect = 0,
        LeftRect = 1,
        TopRect = 1 << 1,
        RightRect = 1 << 2,
        BottomRect = 1 << 3,
        LeftTopRect = LeftRect | TopRect,
        LeftBottomRect = LeftRect | BottomRect,
        RightTopRect = RightRect | TopRect,
        RightBottomRect = RightRect | BottomRect,
    };

    enum CollectionFrameState {
        UnKnowState = -1,
        StretchState,
        MoveState,
        NormalShowState
    };

    explicit CollectionFramePrivate(CollectionFrame *qq = nullptr);
    ~CollectionFramePrivate();

    void updateStretchRect();
    void updateMoveRect();
    ResponseArea getCurrentResponseArea(const QPoint &pos) const;
    void updateCursorState(const ResponseArea &stretchPlace);
    void updateMouseTrackingState();
    void updateFrameGeometry();

    QPoint moveResultRectPos(bool *validPos = nullptr);
    QRect stretchResultRect();
    Surface *surface();

    inline bool canMove();
    bool canStretch();

private:
    int calcLeftX();
    int calcRightX();
    int calcTopY();
    int calcBottomY();

public:
    CollectionFrame *const q = nullptr;
    QWidget *widget = nullptr;
    QWidget *titleBarWidget = nullptr;
    QWidget *collView = nullptr;

    QVBoxLayout *mainLayout = nullptr;
    QRect titleBarRect;
    QList<QRect> stretchRects;
    QPoint stretchEndPoint;
    QRect oldGeometry;
    ResponseArea responseArea = UnKnowRect;
    QPoint moveStartPoint;
    QPoint dragPos;
    QList<ResponseArea> stretchArea;
    QList<ResponseArea> moveArea;
    CollectionFrameState frameState = NormalShowState;
    Surface *oldSurface { nullptr };

    CollectionFrame::CollectionFrameFeatures frameFeatures = CollectionFrame::NoCollectionFrameFeatures;
    CollectionFrame::CollectionFrameStretchStyle stretchStyle = CollectionFrame::CollectionFrameStretchUnLimited;
};

}

#endif   // COLLECTIONFRAME_P_H
