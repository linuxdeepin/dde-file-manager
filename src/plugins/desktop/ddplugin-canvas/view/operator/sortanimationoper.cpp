// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sortanimationoper.h"
#include "canvasmanager.h"
#include "grid/canvasgrid.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::AnimationConfig;

Q_GLOBAL_STATIC(SortAnimationOper, sortAnimationOperGlobal)

SortAnimationOper *SortAnimationOper::instance()
{
    return sortAnimationOperGlobal;
}

SortAnimationOper::SortAnimationOper(QObject *parent)
    : QObject(parent)
{
    moveDelayTimer.setInterval(100);
    moveDelayTimer.setSingleShot(true);

    connect(&moveDelayTimer, &QTimer::timeout, this, &SortAnimationOper::startMoveAnimation);
}

void SortAnimationOper::setMoveValue(const QStringList &moveItems)
{
    if (moveAnimationing)
        return;

    if (moveItems.isEmpty()) {
        fmDebug() << "Empty move items list - ignoring";
        return;
    }

    this->moveItems = moveItems;
    originPos.clear();
    itemsPixmap.clear();
    prepareMove = false;

    for (const QString &item : moveItems) {
        GridPos pos;
        if (GridIns->point(item, pos))
            originPos.insert(item, pos);
    }
}

void SortAnimationOper::setItemPixmap(const QString &item, const QPixmap &pix, int screenNum)
{
    if (item.isEmpty() || pix.isNull())
        return;

    itemsPixmap[qMakePair(item, screenNum)] = pix;
}

QPixmap SortAnimationOper::findPixmap(const QString &item, int screenNum) const
{
    auto key = qMakePair(item, screenNum);
    return itemsPixmap.contains(key) ? itemsPixmap.value(key) : QPixmap();
}

bool SortAnimationOper::tryMove(const QStringList &existItems)
{
    if (moveAnimationing) {
        fmDebug() << "Move animation already in progress - ignoring";
        return false;
    }

    if (moveItems.isEmpty() || originPos.isEmpty()) {
        fmDebug() << "No move items or invalid origin position";
        return false;
    }

    if (!calcMoveTargetGrid(existItems)) {
        fmDebug() << "No moved items after sorting";
        return false;
    }

    prepareMove = true;
    startDelayMove();
    return true;
}

bool SortAnimationOper::getMoveItemGridPos(const QString &item, GridPos &gridPos)
{
    if (!oper.get())
        return false;

    return oper->position(item, gridPos);
}

bool SortAnimationOper::getOriginItemGridPos(const QString &item, GridPos &gridPos) const
{
    if (!originPos.contains(item))
        return false;

    gridPos = originPos.value(item);
    return true;
}

void SortAnimationOper::setMoveDuration(double duration)
{
    if (qFuzzyCompare(moveDuration, duration))
        return;

    moveDuration = duration;
}

void SortAnimationOper::startDelayMove()
{
    moveDelayTimer.start();
}

void SortAnimationOper::stopDelayMove()
{
    moveDelayTimer.stop();
}

void SortAnimationOper::startMoveAnimation()
{
    moveAnimationing = true;
    prepareMove = false;

    if (animation.get())
        animation->disconnect();

    animation.reset(new QPropertyAnimation(this, "moveDuration"));
    int duration;
    QEasingCurve::Type curve;
    switch (GridIns->mode()) {
    case CanvasGrid::Mode::Custom:
        duration = DConfigManager::instance()->value(kAnimationDConfName, kAnimationResortCustomDuration, true).toInt();
        curve = static_cast<QEasingCurve::Type>(DConfigManager::instance()->value(kAnimationDConfName, kAnimationResortCustomCurve).toInt());
        fmDebug() << "Custom mode animation - duration:" << duration << "curve:" << static_cast<int>(curve);
        break;
    case CanvasGrid::Mode::Align:
        duration = DConfigManager::instance()->value(kAnimationDConfName, kAnimationResortAlignDuration, true).toInt();
        curve = static_cast<QEasingCurve::Type>(DConfigManager::instance()->value(kAnimationDConfName, kAnimationResortAlignCurve).toInt());
        fmDebug() << "Align mode animation - duration:" << duration << "curve:" << static_cast<int>(curve);
        break;
    default:
        duration = 366;
        break;
    }
    animation->setDuration(duration);
    animation->setEasingCurve(curve);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);

    connect(animation.get(), &QPropertyAnimation::valueChanged,
            this, &SortAnimationOper::moveAnimationUpdate);
    connect(animation.get(), &QPropertyAnimation::finished,
            this, &SortAnimationOper::moveAnimationFinished);

    animation->start();
}

void SortAnimationOper::moveAnimationUpdate()
{
    CanvasIns->update();
}

void SortAnimationOper::moveAnimationFinished()
{
    moveAnimationing = false;
    CanvasIns->update();

    if (!oper.get()) {
        fmWarning() << "No grid operation available after animation finished";
        return;
    }

    GridIns->core().applay(oper.get());
    GridIns->requestSync();
    itemsPixmap.clear();
    originPos.clear();
    moveItems.clear();
    oper.reset();
}

bool SortAnimationOper::calcMoveTargetGrid(const QStringList &existItems)
{
    oper.reset(new SortItemsOper(&GridIns->core()));
    QStringList targetItems = existItems;
    oper->tryMove(moveItems, targetItems);

    for (const QString &item : moveItems) {
        GridPos from;
        GridPos to;
        if (!getOriginItemGridPos(item, from) || !oper->position(item, to))
            continue;
        if (from != to)
            return true;
    }

    oper.reset();
    return false;
}

SortItemsOper::SortItemsOper(GridCore *core)
    : GridCore(*core)
{
}

void SortItemsOper::tryMove(const QStringList &orgItems, QStringList &movedItems)
{
    clean();

    for (int idx : surfaceIndex()) {
        QHash<QPoint, QString> allPos;
        QHash<QString, QPoint> allItem;
        if (!movedItems.isEmpty()) {
            int max = gridCount(idx);
            const int height = surfaces.value(idx).height();
            int cur = 0;
            for (; cur < max && !movedItems.isEmpty(); ++cur) {
                QString &&item = movedItems.takeFirst();
                QPoint pos(cur / height, cur % height);
                allPos.insert(pos, item);
                allItem.insert(item, pos);
            }
        }

        itemPos.insert(idx, allItem);
        posItem.insert(idx, allPos);
    }

    overload = movedItems;
}

void SortItemsOper::clean()
{
    posItem.clear();
    itemPos.clear();
    overload.clear();
}

int SortItemsOper::gridCount(int index) const
{
    int count = 0;
    if (index < 0) {
        for (auto itor = surfaces.begin(); itor != surfaces.end(); ++itor)
            count += itor.value().width() * itor.value().height();
    } else {
        auto size = surfaceSize(index);
        count = size.width() * size.height();
    }

    return count;
}
