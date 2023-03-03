// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLECTIONFRAME_P_H
#define COLLECTIONFRAME_P_H

#include "collectionframe.h"

#include <QVBoxLayout>

namespace ddplugin_organizer {

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

    void updateStretchRect();
    void updateMoveRect();
    ResponseArea getCurrentResponseArea(const QPoint &pos) const;
    void updateCursorState(const ResponseArea &stretchPlace);
    void updateMouseTrackingState();
    void updateFrameGeometry();

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

    QVBoxLayout *mainLayout = nullptr;
    QRect titleBarRect;
    int minWidth = 20;
    int minHeight = 20;
    QList<QRect> stretchRects;
    QPoint stretchEndPoint;
    QRect stretchBeforRect;
    ResponseArea responseArea = UnKnowRect;
    QPoint moveStartPoint;
    QList<ResponseArea> stretchArea;
    QList<ResponseArea> moveArea;
    CollectionFrameState frameState = NormalShowState;

    CollectionFrame::CollectionFrameFeatures frameFeatures = CollectionFrame::NoCollectionFrameFeatures;
    CollectionFrame::CollectionFrameStretchStyle stretchStyle = CollectionFrame::CollectionFrameStretchUnLimited;
};

}

#endif // COLLECTIONFRAME_P_H
