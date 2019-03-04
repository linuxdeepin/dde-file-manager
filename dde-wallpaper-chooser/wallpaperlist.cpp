/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "wallpaperlist.h"
#include "wallpaperitem.h"
#include "constants.h"

#include <QDebug>
#include <QScrollBar>
#include <QToolButton>
#include <QGSettings>
#include <QHBoxLayout>

#include <danchors.h>
#include <dimagebutton.h>

WallpaperList::WallpaperList(QWidget * parent)
    : QScrollArea(parent)
    , m_wmInter(new com::deepin::wm("com.deepin.wm", "/com/deepin/wm", QDBusConnection::sessionBus(), this))
    , prevButton(new DImageButton(":/images/previous_normal.svg",
                                  ":/images/previous_hover.svg",
                                  ":/images/previous_press.svg", this))
    , nextButton(new DImageButton(":/images/next_normal.svg",
                                  ":/images/next_hover.svg",
                                  ":/images/next_press.svg", this))
    , m_updateTimer(new QTimer(this))
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("WallpaperList{background: transparent;}");
    setFrameShape(QFrame::NoFrame);
    horizontalScrollBar()->setEnabled(false);

    m_updateTimer->setInterval(100);
    m_updateTimer->setSingleShot(true);

    connect(m_updateTimer, &QTimer::timeout, this, &WallpaperList::updateItemThumb);
    connect(&scrollAnimation, &QAbstractAnimation::finished, m_updateTimer, static_cast<void (QTimer::*)()>(&QTimer::start));

    m_contentWidget = new QWidget(this);

    m_contentWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_contentLayout = new QHBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(0);

    setWidget(m_contentWidget);
    m_contentWidget->setAutoFillBackground(false);

    prevButton->hide();
    prevButton.setAnchor(Qt::AnchorVerticalCenter, this, Qt::AnchorVerticalCenter);
    prevButton.setAnchor(Qt::AnchorLeft, this, Qt::AnchorLeft);
    nextButton->hide();
    nextButton.setAnchor(Qt::AnchorVerticalCenter, this, Qt::AnchorVerticalCenter);
    nextButton.setAnchor(Qt::AnchorRight, this, Qt::AnchorRight);

    connect(prevButton.widget(), &DImageButton::clicked, this, &WallpaperList::prevPage);
    connect(nextButton.widget(), &DImageButton::clicked, this, &WallpaperList::nextPage);

    scrollAnimation.setTargetObject(horizontalScrollBar());
    scrollAnimation.setPropertyName("value");
    scrollAnimation.setDuration(QEasingCurve::OutExpo);
}

WallpaperList::~WallpaperList()
{

}

WallpaperItem * WallpaperList::addWallpaper(const QString &path)
{
    WallpaperItem * wallpaper = new WallpaperItem(this, path);
    wallpaper->setFixedSize(QSize(ItemWidth, ItemHeight));
    addItem(wallpaper);

    connect(wallpaper, &WallpaperItem::pressed, this, &WallpaperList::wallpaperItemPressed);
    connect(wallpaper, &WallpaperItem::hoverIn, this, &WallpaperList::wallpaperItemHoverIn);
    connect(wallpaper, &WallpaperItem::hoverOut, this, &WallpaperList::wallpaperItemHoverOut);

    return wallpaper;
}

void WallpaperList::removeWallpaper(const QString &path)
{
    for (int i(0); i < m_items.count(); ++i) {
        WallpaperItem *item = m_items[i];

        if (item->getPath() == path) {
            m_items.removeOne(item);
            m_contentLayout->removeWidget(item);
            item->deleteLater();
            break;
        }
    }

    m_updateTimer->start();
}

void WallpaperList::scrollList(int step, int duration)
{
    if (scrollAnimation.state() == QAbstractAnimation::Running)
        return;

    int start_value = horizontalScrollBar()->value();
    int end_value = start_value + step;

    if ((end_value < horizontalScrollBar()->minimum() && start_value == horizontalScrollBar()->minimum())
            || (end_value > horizontalScrollBar()->maximum() && start_value == horizontalScrollBar()->maximum())) {
        return;
    }

    scrollAnimation.setDuration(duration);
    scrollAnimation.setStartValue(start_value);
    scrollAnimation.setEndValue(end_value);
    scrollAnimation.start();

    prevButton->hide();
    nextButton->hide();

    if (prevItem)
        prevItem->setOpacity(1);

    if (nextItem)
        nextItem->setOpacity(1);

    // hide the delete button.
    emit mouseOverItemChanged("", QPoint(0, 0));
}

void WallpaperList::prevPage()
{
    int c = width() / gridSize().width();

    scrollList((2 - c) * (m_contentLayout->spacing() + ItemWidth), 500);
}

void WallpaperList::nextPage()
{
    int c = width() / gridSize().width();

    scrollList((c - 2) * (m_contentLayout->spacing() + ItemWidth), 500);
}

void WallpaperList::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);

    int screen_item_count = width() / ItemWidth;

    if (width() % ItemWidth == 0)
        --screen_item_count;

    setGridSize(QSize(width() / screen_item_count, ItemHeight));
}

void WallpaperList::wheelEvent(QWheelEvent *event)
{
    Q_UNUSED(event);
}

void WallpaperList::showEvent(QShowEvent *event)
{
    updateBothEndsItem();

    return QScrollArea::showEvent(event);
}

QSize WallpaperList::gridSize() const
{
    return m_gridSize;
}

void WallpaperList::setGridSize(const QSize &size)
{
    if (m_gridSize == size)
        return;

    int c = width() / size.width();

    m_gridSize = size;
    m_contentLayout->setSpacing(qRound((width() - c * ItemWidth) / qreal(c + 1) - 0.500001) + 1);
    m_contentLayout->setContentsMargins(m_contentLayout->spacing(), 0,
                                        m_contentLayout->spacing(), 0);
    m_contentWidget->adjustSize();
}

void WallpaperList::addItem(WallpaperItem *item)
{
    m_items << item;

    m_contentLayout->addWidget(item);
    m_contentWidget->adjustSize();
}

QWidget *WallpaperList::item(int index) const
{
    if (index >= m_contentLayout->count())
        return 0;

    return m_contentLayout->itemAt(index)->widget();
}

QWidget *WallpaperList::itemAt(const QPoint &pos) const
{
    return itemAt(pos.x(), pos.y());
}

QWidget *WallpaperList::itemAt(int x, int y) const
{
    Q_UNUSED(y)

    return item((horizontalScrollBar()->value() + x) / gridSize().width());
}

int WallpaperList::count() const
{
    return m_items.size();
}

void WallpaperList::clear()
{
    for (WallpaperItem * item : m_items) {
        m_contentLayout->removeWidget(item);
        item->deleteLater();
    }

    m_items.clear();
    prevItem = nextItem = nullptr;
}

void WallpaperList::updateItemThumb()
{
    m_contentWidget->adjustSize();

    showDeleteButtonForItem(static_cast<WallpaperItem*>(itemAt(mapFromGlobal(QCursor::pos()))));

    for (WallpaperItem *item : m_items) {
        if (rect().intersects(QRect(item->mapTo(this, QPoint()), item->size()))) {
            item->initPixmap();
        }
    }

    updateBothEndsItem();
}

void WallpaperList::wallpaperItemPressed()
{
    WallpaperItem * item = qobject_cast<WallpaperItem*>(sender());

    if (item == prevItem || item == nextItem)
        return;

    for (int i = 0; i < count(); i++) {
        WallpaperItem * wallpaper = qobject_cast<WallpaperItem*>(this->item(i));

        if (wallpaper) {
            if (wallpaper == item) {
                wallpaper->slideUp();

                emit itemPressed(wallpaper->data());
            } else {
                wallpaper->slideDown();
            }
        }
    }
}

void WallpaperList::wallpaperItemHoverIn()
{
    WallpaperItem *item = qobject_cast<WallpaperItem*>(sender());

    if (item->isVisible())
        showDeleteButtonForItem(item);
}

void WallpaperList::wallpaperItemHoverOut()
{
//    emit needCloseButton("", QPoint(0, 0));
}

void WallpaperList::updateBothEndsItem()
{
    int current_value = horizontalScrollBar()->value();

    if (prevItem)
        prevItem->setOpacity(1);

    if (nextItem)
        nextItem->setOpacity(1);

    prevItem = qobject_cast<WallpaperItem*>(itemAt(ItemWidth / 2, ItemHeight / 2));
    nextItem = qobject_cast<WallpaperItem*>(itemAt(width() - ItemWidth / 2, ItemHeight / 2));

    if (current_value == horizontalScrollBar()->minimum()) {
        prevItem = Q_NULLPTR;
    }

    if (current_value == horizontalScrollBar()->maximum()) {
        nextItem = Q_NULLPTR;
    }

    if (prevItem) {
        prevButton.setLeftMargin(gridSize().width() / 2 - prevButton->sizeHint().width() / 2);
        prevItem->setOpacity(0.4);
    }

    prevButton->setVisible(prevItem);

    if (nextItem) {
        nextButton.setRightMargin(gridSize().width() / 2 - nextButton->sizeHint().width() / 2);
        nextItem->setOpacity(0.4);
    }

    nextButton->setVisible(nextItem);
}

void WallpaperList::showDeleteButtonForItem(WallpaperItem const *item) const
{
    if (item && item->getDeletable()) {
        // we can't get a correct item if content image geometry is not available to use.
        if (item->contentImageGeometry().isNull()) {
            return;
        }
        emit mouseOverItemChanged(item->getPath(),
                             item->mapTo(parentWidget(),
                                         item->contentImageGeometry().topRight() / devicePixelRatioF()));
    } else {
        emit mouseOverItemChanged("", QPoint(0, 0));
    }
}
