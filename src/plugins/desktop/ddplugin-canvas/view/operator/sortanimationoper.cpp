// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sortanimationoper.h"
#include "canvasmanager.h"
#include "grid/canvasgrid.h"
#include "model/canvasproxymodel.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::AnimationConfig;

SortAnimationOper::SortAnimationOper(CanvasView *parent)
    : QObject(parent), view(parent)
{
    moveDelayTimer.setInterval(100);
    moveDelayTimer.setSingleShot(true);

    connect(&moveDelayTimer, &QTimer::timeout, this, &SortAnimationOper::startMoveAnimation);
}

void SortAnimationOper::setMoveValue(const QStringList &moveItems)
{
    if (moveItems.isEmpty()) {
        fmDebug() << "Empty move items list - ignoring";
        return;
    }

    this->moveItems = moveItems;
}

void SortAnimationOper::setItemPixmap(const QString &item, const QPixmap &pix)
{
    if (item.isEmpty() || pix.isNull())
        return;

    itemsPixmap[item] = pix;
}

QPixmap SortAnimationOper::findPixmap(const QString &item) const
{
    return itemsPixmap.contains(item) ? itemsPixmap.value(item) : QPixmap();
}

void SortAnimationOper::tryMove()
{
    if (moveAnimationing) {
        fmDebug() << "Move animation already in progress - ignoring";
        return;
    }

    QPair<int, QPoint> originPos;
    if (moveItems.isEmpty() || !GridIns->point(moveItems.first(), originPos)) {
        fmDebug() << "No move items or invalid origin position";
        return;
    }

    if (originPos.first == view->screenNum())
        startDelayMove();
}

bool SortAnimationOper::getMoveItemGridPos(const QString &item, GridPos &gridPos)
{
    if (!oper.get())
        return false;

    return oper->position(item, gridPos);
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

    calcMoveTargetGrid();
    itemsPixmap.clear();

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
}

void SortAnimationOper::calcMoveTargetGrid()
{
    QStringList existItems;
    const QList<QUrl> &actualList = view->model()->files();
    for (const QUrl &df : actualList)
        existItems.append(df.toString());

    oper.reset(new SortItemsOper(&GridIns->core()));
    oper->tryMove(moveItems, existItems);
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
