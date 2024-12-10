// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "viewanimationhelper.h"
#include "dfmplugin_workspace_global.h"
#include "views/fileview.h"
#include "views/baseitemdelegate.h"
#include "models/fileviewmodel.h"
#include "utils/itemdelegatehelper.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QTimer>
#include <QPainter>
#include <QLabel>
#include <QPropertyAnimation>

DFMBASE_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::AnimationConfig;

ViewAnimationHelper::ViewAnimationHelper(FileView *parent)
    : QObject(parent),
      view(parent)
{
}

void ViewAnimationHelper::initAnimationHelper()
{
    if (!DConfigManager::instance()->value(kAnimationDConfName, kAnimationLayoutEnable, true).toBool())
        return;

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

    if (playingAnim)
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

    if (playingAnim)
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

    resetExpandItem();
    delayTimer->start();
}

bool ViewAnimationHelper::isAnimationPlaying() const
{
    if (animPtr && animPtr->state() == QPropertyAnimation::Running)
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

void ViewAnimationHelper::playAnimationWithWidthChange(int deltaWidth)
{
    if (!initialized)
        return;

    playingAnim = true;

    syncVisiableRect();
    QRect validRect = currentVisiableRect;
    currentIndexRectMap = calcIndexRects(validRect);
    createPixmapsForVisiableRect();

    oldIndexRectMap = currentIndexRectMap;
    currentVisiableRect.setWidth(currentVisiableRect.width() + deltaWidth);
    newIndexRectMap = calcIndexRects(currentVisiableRect);

    resetExpandItem();
    paintPixmaps(newIndexRectMap);

    resetAnimation();
    animPtr->start();
}

void ViewAnimationHelper::paintItems() const
{
    QPainter painter(view->viewport());
    painter.setRenderHint(QPainter::Antialiasing, true);
    auto itemIterator = indexPixmaps.begin();

    auto expandItemRect = QRect();
    auto calcExpandItemRect = [&](const QModelIndex &index, const QRect &rect) {
        if (expandItemIndex.isValid() && index == expandItemIndex) {
            expandItemRect = rect;
            expandItemRect.setTopLeft(rect.topLeft() + expandItemOffset);
            expandItemRect.setSize(expandItemPixmap.size());
        }
    };
    if (isWaitingToPlaying()) {
        while (itemIterator != indexPixmaps.end()) {
            auto index = itemIterator.key();

            if (!currentIndexRectMap.contains(index)) {
                itemIterator++;
                continue;
            }

            QRect paintRect = currentIndexRectMap[index];
            calcExpandItemRect(index, paintRect);

            QPixmap paintPix = itemIterator.value();
            painter.drawPixmap(paintRect, paintPix);

            itemIterator++;
        }
    } else if (animPtr && animPtr->state() == QPropertyAnimation::Running) {
        QSize viewportSize = view->contentsSize();
        QPoint itemBirthPos(viewportSize.width() / 2, viewportSize.height());

        while (itemIterator != indexPixmaps.end()) {
            auto index = itemIterator.key();

            QPoint targetPos = newIndexRectMap[index].topLeft();
            QPoint oldPos;
            if (!oldIndexRectMap.contains(index)) {
                oldPos = itemBirthPos;
            } else {
                oldPos = oldIndexRectMap[index].topLeft();
            }

            int deltaX = (targetPos.x() - oldPos.x()) * animProcess;
            int deltaY = (targetPos.y() - oldPos.y()) * animProcess;

            QPoint newPos(oldPos.x() + deltaX, oldPos.y() + deltaY);

            QRect paintRect = newIndexRectMap[index];
            paintRect.moveTopLeft(newPos);

            calcExpandItemRect(index, paintRect);

            painter.drawPixmap(paintRect, itemIterator.value());

            itemIterator++;
        }
    }

    if (expandItemRect.isValid())
        painter.drawPixmap(expandItemRect, expandItemPixmap);
}

void ViewAnimationHelper::setAnimProcess(double value)
{
    if (qFuzzyCompare(animProcess, value))
        return;

    animProcess = value;
}

void ViewAnimationHelper::onDelayTimerFinish()
{
    oldIndexRectMap = currentIndexRectMap;

    syncVisiableRect();
    newIndexRectMap = calcIndexRects(currentVisiableRect);

    paintPixmaps(newIndexRectMap);

    resetAnimation();

    animPtr->start();
}

void ViewAnimationHelper::onAnimationValueChanged()
{
    view->viewport()->update();
}

void ViewAnimationHelper::onAnimationTimerFinish()
{
    view->viewport()->update();
    playingAnim = false;
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

        qreal scale = view->devicePixelRatioF();
        QPixmap pixmap(rect.size() * scale);
        pixmap.setDevicePixelRatio(scale);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
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

void ViewAnimationHelper::resetAnimation()
{
    if (!animPtr) {
        animPtr = new QPropertyAnimation(this, "animProcess", this);

        int duration = DConfigManager::instance()->value(kAnimationDConfName, kAnimationLayoutDuration, 366).toInt();
        auto curve = static_cast<QEasingCurve::Type>(DConfigManager::instance()->value(kAnimationDConfName, kAnimationLayoutCurve).toInt());
        animPtr->setDuration(duration);
        animPtr->setEasingCurve(curve);
        animPtr->setStartValue(0.0);
        animPtr->setEndValue(1.0);

        connect(animPtr, &QPropertyAnimation::valueChanged, this, &ViewAnimationHelper::onAnimationValueChanged);
        connect(animPtr, &QPropertyAnimation::finished, this, &ViewAnimationHelper::onAnimationTimerFinish);
    }

    if (animPtr->state() == QPropertyAnimation::Running)
        animPtr->stop();
}

void ViewAnimationHelper::resetExpandItem()
{
    if (view->selectedIndexes().count() != 1) {
        expandItemIndex = QModelIndex();
        expandItemPixmap = QPixmap();
        return;
    }

    if (view->itemDelegate()->itemExpanded()) {
        QWidget *expandedItem = view->itemDelegate()->expandedItem();
        if (!expandedItem)
            return;

        expandItemIndex = view->itemDelegate()->expandedIndex();
        auto itemRect = expandedItem->rect();
        itemRect.moveTopLeft(QPoint(0, 0));
        auto itemIconRect = view->itemDelegate()->itemIconRect(itemRect);
        auto itemLabelRect = itemRect.adjusted(0, itemIconRect.height() + kIconModeTextPadding + kIconModeIconSpacing, 0, 0);
        expandItemOffset = itemLabelRect.topLeft();
        expandItemPixmap = expandedItem->grab(itemLabelRect);
    }
}

