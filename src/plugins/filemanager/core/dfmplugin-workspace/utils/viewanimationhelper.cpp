// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "viewanimationhelper.h"
#include "dfmplugin_workspace_global.h"
#include "views/fileview.h"
#include "views/baseitemdelegate.h"
#include "models/fileviewmodel.h"

#include <QTimer>
#include <QPainter>
#include <QLabel>

#include <math.h>

DPWORKSPACE_USE_NAMESPACE

ViewAnimationHelper::ViewAnimationHelper(FileView *parent)
    : QObject(parent),
      view(parent)
{
}

void ViewAnimationHelper::initAnimationHelper()
{
    currentIndexRectMap = calcIndexRects(view->contentsRect());
    initialized = true;
}

void ViewAnimationHelper::reset()
{
    currentIndexRectMap.clear();
    initialized = false;
}

void ViewAnimationHelper::syncVisiableRect()
{
    currentVisiableRect = view->viewport()->rect();
    currentVisiableRect.moveTop(view->verticalOffset());
}

void ViewAnimationHelper::aboutToPlay()
{
    if (isWaitingToPlaying())
        return;

    oldVisiableRect = view->viewport()->rect();
    oldVisiableRect.moveTop(view->verticalOffset());
    indexPixmaps.clear();
}

QRect ViewAnimationHelper::getCurrentRectByIndex(const QModelIndex &index) const
{
    if (currentIndexRectMap.contains(index))
        return currentIndexRectMap[index];

    return QRect();
}

void ViewAnimationHelper::playViewAnimation()
{
    if (!initialized)
        return;

    if (!delayTimer) {
        delayTimer = new QTimer(this);
        delayTimer->setSingleShot(true);
        delayTimer->setInterval(100);
        connect(delayTimer, &QTimer::timeout, this, &ViewAnimationHelper::onDelayTimerFinish);
    }

    if (!delayTimer->isActive())
        newIndexRectMap.clear();

    syncVisiableRect();
    QRect validRect = currentVisiableRect;
    validRect.setWidth(oldVisiableRect.width());
    currentIndexRectMap = calcIndexRects(validRect);
    createPixmapsForVisiableRect();

    delayTimer->start();
}

bool ViewAnimationHelper::isAnimationPlaying() const
{
    if (animationTimer && animationTimer->isActive())
        return true;

    return false;
}

bool ViewAnimationHelper::isWaitingToPlaying() const
{
    if (delayTimer && delayTimer->isActive())
        return true;

    return false;
}

bool ViewAnimationHelper::hasInitialized() const
{
    return initialized;
}

void ViewAnimationHelper::paintItems() const
{
    QPainter painter(view->viewport());
    auto itemIterator = indexPixmaps.begin();

    if (isWaitingToPlaying()) {
        while (itemIterator != indexPixmaps.end()) {
            auto index = itemIterator.key();

            if (!currentIndexRectMap.contains(index)) {
                itemIterator++;
                continue;
            }

            QRect paintRect = currentIndexRectMap[index];
            painter.drawPixmap(paintRect, itemIterator.value());

            itemIterator++;
        }
    } else if (isAnimationPlaying()) {
        QSize viewportSize = view->contentsSize();
        QPoint itemBirthPos(viewportSize.width() / 2, viewportSize.height());

        int elapse = animFrame * kViewAnimationFrameDuration;

        while (itemIterator != indexPixmaps.end()) {
            auto index = itemIterator.key();

            QPoint targetPos = newIndexRectMap[index].topLeft();
            QPoint oldPos;
            if (!oldIndexRectMap.contains(index)) {
                oldPos = itemBirthPos;
            } else {
                oldPos = oldIndexRectMap[index].topLeft();
            }

            qreal proccess = static_cast<qreal>(elapse) / static_cast<qreal>(kViewAnimationDuration);
            int deltaX = (targetPos.x() - oldPos.x()) * easeOutExpo(proccess);
            int deltaY = (targetPos.y() - oldPos.y()) * easeOutExpo(proccess);

            QPoint newPos(oldPos.x() + deltaX, oldPos.y() + deltaY);

            QRect paintRect = newIndexRectMap[index];
            paintRect.moveTopLeft(newPos);

            painter.drawPixmap(paintRect, itemIterator.value());

            itemIterator++;
        }
    }
}

void ViewAnimationHelper::onDelayTimerFinish()
{
    if (!animationTimer) {
        animationTimer = new QTimer(this);
        animationTimer->setInterval(kViewAnimationFrameDuration);
        animationTimer->setSingleShot(false);
        connect(animationTimer, &QTimer::timeout, this, &ViewAnimationHelper::onAnimationTimerFinish);
    }

    if (animationTimer->isActive())
        animationTimer->stop();

    animFrame = 0;
    animationTimer->start();
}

void ViewAnimationHelper::onAnimationTimerFinish()
{
    if (animFrame == 0) {
        oldIndexRectMap = currentIndexRectMap;

        syncVisiableRect();
        newIndexRectMap = calcIndexRects(currentVisiableRect);

        paintPixmaps(newIndexRectMap);
    }

    int elapse = ++animFrame * kViewAnimationFrameDuration;
    if (elapse > kViewAnimationDuration)
        animationTimer->stop();

    view->viewport()->update();
}

QMap<QModelIndex, QRect> ViewAnimationHelper::calcIndexRects(const QRect &rect) const
{
    auto visibleIndexes = view->visibleIndexes(rect);

    QMap<QModelIndex, QRect> map {};
    for (auto rangeList : visibleIndexes) {
        for (int i = rangeList.first; i <= rangeList.second; ++i) {
            auto index = view->model()->index(i, 0, view->model()->rootIndex());
            auto itemRect = view->calcVisualRect(rect.width(), i);
            map[index] = itemRect;
        }
    }

    return map;
}

void ViewAnimationHelper::paintPixmaps(const QMap<QModelIndex, QRect> &indexRects)
{
    auto selectIndexes = view->selectedIndexes();
    for (auto index : indexRects.keys()) {
        if (!index.isValid() || indexPixmaps.contains(index))
            continue;

        const qreal scale = view->devicePixelRatioF();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QStyleOptionViewItem option = view->viewOptions();
#else
        QStyleOptionViewItem option;
        view->initViewItemOption(&option);
#endif
        if (selectIndexes.contains(index))
            option.state |= QStyle::State_Selected;

        // NOTE: use State_AutoRaise flag to make decide if paint item pixmap for view animation in iconitemdelegate.
        // refer code in iconitemdelegate.cpp function paintItemFileName().
        option.state |= QStyle::State_AutoRaise;

        QRect rect = indexRects[index];
        rect.moveTopLeft(QPoint(0, 0));
        option.rect = rect;

        QPixmap pixmap(rect.size());
        pixmap.setDevicePixelRatio(scale);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        view->itemDelegate()->paint(&painter, option, index);

        indexPixmaps[index] = pixmap;
    }
}

void ViewAnimationHelper::createPixmapsForVisiableRect()
{
    QRect validRect = currentVisiableRect;
    validRect.setWidth(oldVisiableRect.width());

    auto visiableIndexRects = calcIndexRects(validRect);
    paintPixmaps(visiableIndexRects);
}

qreal ViewAnimationHelper::easeOutExpo(qreal value) const
{
    return value == 1.0 ? 1.0 : 1.0 - pow(2, -10 * value);
}
