/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *             xinglinkun<xinglinkun@uniontech.com>
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
#include <QHBoxLayout>

#include <danchors.h>
#include <dimagebutton.h>

WallpaperList::WallpaperList(QWidget *parent)
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
    this->setObjectName("WallpaperList-QScrollArea");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAttribute(Qt::WA_TranslucentBackground);
    setFrameShape(QFrame::NoFrame);
    viewport()->setAutoFillBackground(false);
    horizontalScrollBar()->setEnabled(false);
    this->setFocusPolicy(Qt::NoFocus);
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

WallpaperItem *WallpaperList::addWallpaper(const QString &path)
{
    WallpaperItem *wallpaper = new WallpaperItem(this, path);
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

            if (item == prevItem) {
                prevItem = nullptr;
            }

            else if (item == nextItem) {
                nextItem = nullptr;
            }

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
    if (gridSize().width() < 1){
        qCritical() << "error gridSize().width() " << gridSize().width();
        return;
    }

    int c = width() / gridSize().width();

    scrollList((2 - c) * (m_contentLayout->spacing() + ItemWidth), 500);
}

void WallpaperList::nextPage()
{
    if (gridSize().width() < 1){
        qCritical() << "error gridSize().width() " << gridSize().width();
        return;
    }

    int c = width() / gridSize().width();
    scrollList((c - 2) * (m_contentLayout->spacing() + ItemWidth), 500);
}

void WallpaperList::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Left:
    case Qt::Key_Right:
        //fix bug#39948,按住不放时需等待切换结束再继续切换
        if (event->isAutoRepeat() && scrollAnimation.state() == QAbstractAnimation::Running){
            event->accept();
            return;
        }
        //选中下一壁纸
        if (event->key() == Qt::Key_Right)
            setCurrentIndex(m_index+1);
        else    //选中上一壁纸
            setCurrentIndex(m_index-1);
        break;
    default:
    //保持按键事件传递
        event->ignore();
        break;
    }
}

void WallpaperList::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    if (width() < ItemWidth){
        qCritical() << "error. widget width is less than ItemWidth" <<
                   width() << "<" << ItemWidth
                    << "resize" << event->size();
    }

    int screen_item_count = width() / ItemWidth;
    if (width() % ItemWidth == 0)
        --screen_item_count;

    if (screen_item_count < 1){
        qCritical() << "screen_item_count: " << screen_item_count
                    << "set to 1";
        screen_item_count = 1;
    }
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
    int c = 0;
    if (width() == 0 || size.width() == 0)
        c = 0;
    else
        c = width() / size.width();

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
    if (index >= m_contentLayout->count() || index < 0){
        qCritical() << "error index" << index << "gridsie" << m_gridSize << this->geometry();
        return nullptr;
    }

    return m_contentLayout->itemAt(index)->widget();
}

QWidget *WallpaperList::itemAt(const QPoint &pos) const
{
    return itemAt(pos.x(), pos.y());
}

QWidget *WallpaperList::itemAt(int x, int y) const
{
    Q_UNUSED(y)
    if (gridSize().width() < 1){
        qCritical() << "error gridSize().width() " << gridSize().width();
        return nullptr;
    }

    return item((horizontalScrollBar()->value() + x) / gridSize().width());
}

int WallpaperList::count() const
{
    return m_items.size();
}

void WallpaperList::clear()
{
    for (WallpaperItem *item : m_items) {
        m_contentLayout->removeWidget(item);
        item->deleteLater();
    }

    m_items.clear();
    prevItem = nextItem = nullptr;
}

void WallpaperList::updateItemThumb()
{
    qDebug() << "items" << m_items.size();
    m_contentWidget->adjustSize();

    showDeleteButtonForItem(static_cast<WallpaperItem *>(itemAt(mapFromGlobal(QCursor::pos()))));

    //fix bug44918 首次打开壁纸库，点击图片切换到下一页图片，过程中会出现短暂空库
    QRect r = rect();
    //判断区域增加前一页，当前页，下一页的壁纸缩略图
    QRect cacheRect(r.x() - r.width(), r.y(),r.width() * 3, r.height());
    for (WallpaperItem *item : m_items) {
        if (cacheRect.intersects(QRect(item->mapTo(this, QPoint()), item->size()))) {
            item->initPixmap();
        }
    }

    updateBothEndsItem();
}

void WallpaperList::setCurrentIndex(int index)
{
    if(index<0 || index>=count())
        return;
    WallpaperItem *item = m_items.at(index);

    for (int i = 0; i < count(); i++) {
        WallpaperItem *wallpaper = qobject_cast<WallpaperItem *>(this->item(i));

        if (wallpaper) {
            if (wallpaper == item) {
                wallpaper->slideUp();

                emit itemPressed(wallpaper->data());
            } else {
                wallpaper->slideDown();
            }
        }
    }

    //动画，保持被选中壁纸、屏保居中
    int visualCount = width() / (ItemWidth + m_contentLayout->spacing()); //计算显示壁纸数
    scrollAnimation.setDuration(500);
    int prevIndex = m_items.indexOf(qobject_cast<WallpaperItem *>(itemAt(ItemWidth / 2, ItemHeight / 2)),0);
    int nextIndex = m_items.indexOf(qobject_cast<WallpaperItem *>(itemAt(width() - ItemWidth / 2, ItemHeight / 2)),0);
    scrollAnimation.setStartValue(((prevIndex+nextIndex)/2-visualCount/2) * (ItemWidth+m_contentLayout->spacing()));
    scrollAnimation.setEndValue((index-visualCount/2) * (ItemWidth+m_contentLayout->spacing()));
    scrollAnimation.start();
    m_index = m_items.indexOf(item, 0);
}

WallpaperItem *WallpaperList::getCurrentItem()
{
    // fix bug42257进入到“壁纸与屏保”界面，按TAB键，桌面崩溃
    if(m_index<0 || m_index>=count()) {
        if (m_items.isEmpty()) {
            return nullptr;
        } else {
            return m_items.first();
        }
    }
    return m_items.at(m_index);
}

void WallpaperList::wallpaperItemPressed()
{
    WallpaperItem *item = qobject_cast<WallpaperItem *>(sender());
    setCurrentIndex(m_items.indexOf(item,0));
}

void WallpaperList::wallpaperItemHoverIn()
{
    WallpaperItem *item = qobject_cast<WallpaperItem *>(sender());

    if (item && item->isVisible())
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

    prevItem = qobject_cast<WallpaperItem *>(itemAt(ItemWidth / 2, ItemHeight / 2));
    nextItem = qobject_cast<WallpaperItem *>(itemAt(width() - ItemWidth / 2, ItemHeight / 2));

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
    if (item && item->getDeletable() && item != prevItem && item != nextItem ) {
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
