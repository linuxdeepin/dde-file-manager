// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SORTANIMATIONOPER_H
#define SORTANIMATIONOPER_H

#include "grid/gridcore.h"
#include "view/canvasview.h"

#include <QAtomicInteger>
#include <QObject>
#include <QPointer>
#include <QPropertyAnimation>
#include <QSharedPointer>
#include <QTimer>

namespace ddplugin_canvas {

class SortItemsOper : public GridCore
{
public:
    explicit SortItemsOper(GridCore *core);
    void tryMove(const QStringList &orgItems, QStringList &movedItems);

protected:
    void clean();
    int gridCount(int index) const;
};

class SortAnimationOper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(double moveDuration READ getMoveDuration WRITE setMoveDuration NOTIFY moveDurationChanged)
public:
    explicit SortAnimationOper(CanvasView *parent);

    void setMoveValue(const QStringList &moveItems);
    void setItemPixmap(const QString &item, const QPixmap &pix);
    QPixmap findPixmap(const QString &item) const;
    void tryMove();

    inline bool getPrepareMove() const { return prepareMove; }
    inline bool getMoveAnimationing() const { return moveAnimationing; }
    inline QStringList getMoveItems() const { return moveItems; }
    inline double getMoveDuration() const { return moveDuration; }

    bool getMoveItemGridPos(const QString &item, GridPos &gridPos);
    void setMoveDuration(double duration);
    void startDelayMove();
    void stopDelayMove();

signals:
    void moveDurationChanged(double duration);

private slots:
    void startMoveAnimation();
    void moveAnimationUpdate();
    void moveAnimationFinished();

private:
    void calcMoveTargetGrid();

private:
    CanvasView *view = nullptr;
    QSharedPointer<SortItemsOper> oper;

    QTimer moveDelayTimer;
    QSharedPointer<QPropertyAnimation> animation;
    QAtomicInteger<bool> prepareMove = false;
    QAtomicInteger<bool> moveAnimationing = false;
    double moveDuration = 0.0;
    QStringList moveItems;
    QMap<QString, QPixmap> itemsPixmap;
};

}   // namespace ddplugin_canvas

#endif   // SORTANIMATIONOPER_H
