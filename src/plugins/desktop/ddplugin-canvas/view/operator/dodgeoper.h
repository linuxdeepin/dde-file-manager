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

DDP_CANVAS_BEGIN_NAMESPACE

class DodgeItemsOper : public GridCore
{
public:
    explicit DodgeItemsOper(GridCore *core);
    bool tryDodge(const QStringList &orgItems, const GridPos &ref, QStringList &dodgeItems);

private:
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

    bool getPrepareDodge() const;
    bool getDodgeAnimationing() const;
    QStringList getDodgeItems() const;
    QPoint getDragTargetGridPos() const;
    double getDodgeDuration() const;
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

DDP_CANVAS_END_NAMESPACE

#endif // DODGEOPER_H
