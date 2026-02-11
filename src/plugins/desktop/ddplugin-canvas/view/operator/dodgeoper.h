// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DODGEOPER_H
#define DODGEOPER_H

#include "ddplugin_canvas_global.h"
#include "view/canvasview.h"
#include "grid/canvasgrid_p.h"

#include <QObject>
#include <QTimer>
#include <QAtomicInteger>
#include <QPropertyAnimation>
#include <QPointer>
#include <QSharedPointer>

namespace ddplugin_canvas {

class DodgeItemsOper : public GridCore
{
public:
    explicit DodgeItemsOper(GridCore *core);
    bool tryDodge(const QStringList &orgItems, const GridPos &ref, QStringList &dodgeItems);

protected:
    int toIndex(const int screenNumber, const QPoint &pos);
    QList<int> toIndex(const int screenNumber, const QList<QPoint> &pos);
    QPoint toPos(const int screenNumber, const int index);

    QStringList reloach(int screenNumber, int targetIndex, int targetBeforNeedEmptyCount, int targetAfterNeedEmptyCount);

    int findEmptyBackward(int screenNum, int index, int targetAfterNeedEmptyCount);
    QStringList reloachBackward(int screenNum, int start, int end);

    int findEmptyForward(int screenNum, int index, int targetBeforNeedEmptyCount);
    QStringList reloachForward(int screenNum, int start, int end);

};

class DodgeOper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(double dodgeDuration READ getDodgeDuration WRITE setDodgeDuration NOTIFY dodgeDurationChanged)
public:
    explicit DodgeOper(CanvasView *parent);
    ~DodgeOper();

    void updatePrepareDodgeValue(QEvent *event);
    void tryDodge(QDragMoveEvent *event);

    inline bool getPrepareDodge() const{
        return prepareDodge;
    }
    inline bool getDodgeAnimationing() const{
        return dodgeAnimationing;
    }
    inline QStringList getDodgeItems() const {
        return dodgeItems;
    }
    inline QPoint getDragTargetGridPos() const {
        return dragTargetGridPos;
    }
    inline double getDodgeDuration() const {
        return dodgeDuration;
    }

    bool getDodgeItemGridPos(const QString &item, GridPos &gridPos);
    void setDodgeDuration(double duration);
    void startDelayDodge();
    void stopDelayDodge();
signals:
    void dodgeDurationChanged(double getDodgeDuration);

private slots:
    void startDodgeAnimation();
    void dodgeAnimationUpdate();
    void dodgeAnimationFinished();
private:
    bool calcDodgeTargetGrid();

private:
    CanvasView *view = nullptr;
    QSharedPointer<DodgeItemsOper> oper;

    QTimer dodgeDelayTimer;
    QSharedPointer<QPropertyAnimation> animation;
    QAtomicInteger<bool> prepareDodge = false;
    QAtomicInteger<bool> dodgeAnimationing = false;
    double dodgeDuration = 0.0;
    QStringList dodgeItems;
    QPoint dragTargetGridPos = QPoint(-1, -1);
};

}

#endif // DODGEOPER_H
