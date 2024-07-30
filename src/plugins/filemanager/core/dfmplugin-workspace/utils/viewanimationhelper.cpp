// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "viewanimationhelper.h"
#include "dfmplugin_workspace_global.h"
#include "views/fileview.h"
#include "views/baseitemdelegate.h"

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

QRect ViewAnimationHelper::getCurrentRectByIndex(const QModelIndex &index) const
{
    if (currentIndexRectMap.contains(index))
        return currentIndexRectMap[index];

    return QRect();
}

void ViewAnimationHelper::syncItemRect(const QModelIndex &index, const QRect &rect)
{
    currentIndexRectMap[index] = rect;
}

void ViewAnimationHelper::setNewItemRect(const QModelIndex &index, const QRect &rect)
{
    newIndexRectMap[index] = rect;
}

bool ViewAnimationHelper::checkColumnChanged(int newCount)
{
    if (currentColumnCount < 0) {
        currentColumnCount = newCount;
        return false;
    }

    if (currentColumnCount == newCount)
        return false;

    currentColumnCount = newCount;
    return true;
}

void ViewAnimationHelper::playViewAnimation()
{
    if (!proposeTimer) {
        proposeTimer = new QTimer(this);
        proposeTimer->setSingleShot(true);
        proposeTimer->setInterval(100);
        connect(proposeTimer, &QTimer::timeout, this, &ViewAnimationHelper::onProposeTimerFinish);
    }

    stopAnimation();
    proposeTimer->start();
}

bool ViewAnimationHelper::isInAnimationProccess() const
{
    if (proposeTimer && proposeTimer->isActive())
        return true;

    if (animationTimer && animationTimer->isActive())
        return true;

    return proposeToPlay;
}

void ViewAnimationHelper::onProposeTimerFinish()
{
    proposeToPlay = false;
    if (!animationTimer) {
        animationTimer = new QTimer(this);
        animationTimer->setInterval(kViewAnimationFrameDuration);
        animationTimer->setSingleShot(false);
        connect(animationTimer, &QTimer::timeout, this, &ViewAnimationHelper::onAnimationTimerFinish);
    }

    if (animationTimer->isActive()) {
        animationTimer->stop();
        clearLabels();
    }

    animFrame = 0;
    animationTimer->start();
}

void ViewAnimationHelper::onAnimationTimerFinish()
{
    if (animFrame == 0) {
        oldIndexRectMap = currentIndexRectMap;

        if (!blankBackground) {
            blankBackground = new QLabel(view);
            blankBackground->setAutoFillBackground(true);
            auto palette = blankBackground->palette();
        }

        auto contentWidget = view->contentWidget();
        blankBackground->resize(contentWidget->size());
        blankBackground->move(contentWidget->rect().topLeft());
        blankBackground->show();

        createItemLabels();
    }

    int elapse = ++animFrame * kViewAnimationFrameDuration;
    if (elapse > kViewAnimationDuration) {
        currentIndexRectMap = newIndexRectMap;
        animationTimer->stop();
        blankBackground->hide();
        clearLabels();
        view->viewport()->update();
        return;
    }

    QPoint itemBirthPos(blankBackground->width() / 2, blankBackground->height());
    auto itemIterator = indexLabels.begin();
    while (itemIterator != indexLabels.end()) {
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

        QLabel *item = itemIterator.value();
        item->move(newPos);

        itemIterator++;
    }
}

void ViewAnimationHelper::stopAnimation()
{
    if (animationTimer)
        animationTimer->stop();

    if (blankBackground)
        blankBackground->hide();

    clearLabels();
    clearData();
}

void ViewAnimationHelper::clearData()
{
    indexLabels.clear();
    newIndexRectMap.clear();
    indexPixmaps.clear();

    proposeToPlay = true;
}

void ViewAnimationHelper::clearLabels()
{
    for (auto labelPtr : indexLabels.values()) {
        delete labelPtr;
    }
    indexLabels.clear();
}

void ViewAnimationHelper::paintPixmaps()
{
    auto selectIndexes = view->selectedIndexes();
    for (auto index : newIndexRectMap.keys()) {
        if (!index.isValid())
            continue;

        const qreal scale = view->devicePixelRatioF();
        QStyleOptionViewItem option = view->viewOptions();

        if (selectIndexes.contains(index))
            option.state |= QStyle::State_Selected;

        // NOTE: use State_AutoRaise flag to make decide if paint item pixmap for view animation in iconitemdelegate.
        // refer code in iconitemdelegate.cpp function paintItemFileName().
        option.state |= QStyle::State_AutoRaise;

        QRect rect = newIndexRectMap[index];
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

void ViewAnimationHelper::createItemLabels()
{
    paintPixmaps();

    auto pixmapIterator = indexPixmaps.begin();
    while (pixmapIterator != indexPixmaps.end()) {
        auto index = pixmapIterator.key();

        QLabel *item = new QLabel(blankBackground);
        item->setPixmap(pixmapIterator.value());
        item->resize(pixmapIterator.value().size());
        if (!oldIndexRectMap.contains(index)) {
            item->move(blankBackground->width() / 2, blankBackground->height());
        } else {
            item->move(oldIndexRectMap[index].topLeft());
        }

        item->show();

        indexLabels[index] = item;

        pixmapIterator++;
    }
}

qreal ViewAnimationHelper::easeOutExpo(qreal value) const
{
    return value == 1.0 ? 1.0 : 1.0 - pow(2, -10 * value);
}
