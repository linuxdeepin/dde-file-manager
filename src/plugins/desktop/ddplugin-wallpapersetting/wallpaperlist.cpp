// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wallpaperlist.h"

#include <QScrollBar>
#include <QToolButton>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE
using namespace ddplugin_wallpapersetting;

const int WallpaperList::kItemWidth = 172;
const int WallpaperList::kItemHeight = 100;

WallpaperList::WallpaperList(QWidget *parent)
    : QScrollArea(parent)
{
    init();
}

WallpaperList::~WallpaperList()
{
    takeWidget();
    delete contentWidget;
    contentWidget = nullptr;
}

void WallpaperList::setMaskWidget(QWidget *w)
{
    if (!w) {
        fmWarning() << "Cannot set mask widget: null widget provided";
        return;
    }

    takeWidget();
    setWidget(w);
    w->setAutoFillBackground(false);   //keep background same as parent
    contentWidget->hide();
}

QWidget *WallpaperList::removeMaskWidget()
{
    QWidget *wid = nullptr;
    if (widget() != contentWidget) {
        wid = takeWidget();
        setWidget(contentWidget);
        contentWidget->setAutoFillBackground(false);   // keep background same as parent
        contentWidget->show();
    }
    return wid;
}

WallpaperItem *WallpaperList::addItem(const QString &itemData)
{
    WallpaperItem *wallpaper = new WallpaperItem(this);
    wallpaper->setItemData(itemData);
    wallpaper->setFixedSize(QSize(kItemWidth, kItemHeight));

    items << wallpaper;

    contentLayout->addWidget(wallpaper);
    contentWidget->adjustSize();

    connect(wallpaper, &WallpaperItem::pressed, this, &WallpaperList::onItemPressed);
    connect(wallpaper, &WallpaperItem::hoverIn, this, &WallpaperList::onItemHoverIn);
    connect(wallpaper, &WallpaperItem::hoverOut, this, &WallpaperList::onItemHoverOut);

    return wallpaper;
}

void WallpaperList::removeItem(const QString &itemData)
{
    for (int i = 0; i < items.count(); ++i) {
        WallpaperItem *item = items[i];
        if (item->itemData() == itemData) {
            if (item == prevItem)
                prevItem = nullptr;
            else if (item == nextItem)
                nextItem = nullptr;

            items.removeOne(item);
            contentLayout->removeWidget(item);
            item->deleteLater();
            break;
        }
    }

    updateTimer->start();
}

QSize WallpaperList::gridSize() const
{
    return grid;
}

void WallpaperList::setGridSize(const QSize &size)
{
    if (grid == size) {
        fmDebug() << "Grid size unchanged, skipping update:" << size;
        return;
    }

    int c = 0;
    if (width() == 0 || size.width() == 0) {
        fmWarning() << "Invalid dimensions for grid calculation, width:" << width() << "grid width:" << size.width();
        c = 0;
    } else {
        c = width() / size.width();
    }

    grid = size;
    contentLayout->setSpacing(qRound((width() - c * kItemWidth) / qreal(c + 1) - 0.500001) + 1);
    contentLayout->setContentsMargins(contentLayout->spacing(), 0,
                                      contentLayout->spacing(), 0);
    contentWidget->adjustSize();
}

QWidget *WallpaperList::itemAt(int x, int y) const
{
    Q_UNUSED(y)
    if (grid.width() < 1) {
        fmCritical() << "error gridSize().width() " << gridSize().width();
        return nullptr;
    }

    return itemAt((horizontalScrollBar()->value() + x) / grid.width());
}

void WallpaperList::clear()
{
    for (WallpaperItem *item : items) {
        contentLayout->removeWidget(item);
        item->deleteLater();
    }

    items.clear();
    prevItem = nextItem = nullptr;
    currentIndex = 0;
}

void WallpaperList::setCurrentIndex(int index)
{
    if (index < 0 || index >= count()) {
        fmWarning() << "Invalid index for setCurrentIndex:" << index << "valid range: 0 -" << (count() - 1);
        return;
    }

    WallpaperItem *item = items.at(index);
    for (int i = 0; i < count(); i++) {
        WallpaperItem *wallpaper = qobject_cast<WallpaperItem *>(itemAt(i));
        if (wallpaper) {
            if (wallpaper == item) {
                wallpaper->slideUp();
                emit itemPressed(wallpaper->itemData());
            } else {
                wallpaper->slideDown();
            }
        }
    }

    int visualCount = width() / (kItemWidth + contentLayout->spacing());
    scrollAnimation.setDuration(500);
    int prevIndex = items.indexOf(qobject_cast<WallpaperItem *>(itemAt(kItemWidth / 2, kItemHeight / 2)), 0);
    int nextIndex = items.indexOf(qobject_cast<WallpaperItem *>(itemAt(width() - kItemWidth / 2, kItemHeight / 2)), 0);
    scrollAnimation.setStartValue(((prevIndex + nextIndex) / 2 - visualCount / 2) * (kItemWidth + contentLayout->spacing()));
    scrollAnimation.setEndValue((index - visualCount / 2) * (kItemWidth + contentLayout->spacing()));

    //the starting direction is opposite to the target direction
    {
        int start = scrollAnimation.startValue().toInt();
        int end = scrollAnimation.endValue().toInt();
        int current = horizontalScrollBar()->value();
        if (((start - end) * (current - start)) < 0) {
            fmDebug() << "the starting direction is opposite to the target direction"
                      << start << end << current << horizontalScrollBar()->maximum();
            scrollAnimation.setStartValue(current);
        }
    }

    scrollAnimation.start();
    currentIndex = items.indexOf(item, 0);
}

WallpaperItem *WallpaperList::currentItem() const
{
    if (currentIndex < 0 || currentIndex >= count()) {
        return items.isEmpty() ? nullptr : items.first();
    }
    return items.at(currentIndex);
}

void WallpaperList::prevPage()
{
    if (gridSize().width() < 1) {
        fmCritical() << "error gridSize().width() " << gridSize().width();
        return;
    }

    int c = width() / gridSize().width();
    scrollList((2 - c) * (contentLayout->spacing() + kItemWidth), 500);
}

void WallpaperList::nextPage()
{
    if (gridSize().width() < 1) {
        fmCritical() << "error gridSize().width() " << gridSize().width();
        return;
    }

    int c = width() / gridSize().width();
    scrollList((c - 2) * (contentLayout->spacing() + kItemWidth), 500);
}

void WallpaperList::onItemPressed(WallpaperItem *it)
{
    if (it)
        setCurrentIndex(items.indexOf(it, 0));
}

void WallpaperList::onItemHoverIn(WallpaperItem *it)
{
    if (it && it->isVisible())
        showDeleteButtonForItem(it);
}

// need to delete
void WallpaperList::onItemHoverOut(WallpaperItem *it)
{
    Q_UNUSED(it)
}

void WallpaperList::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right:
        if (event->isAutoRepeat() && QAbstractAnimation::Running == scrollAnimation.state()) {
            event->accept();
            return;
        }

        if (Qt::Key_Left == event->key()) {
            setCurrentIndex(currentIndex - 1);
        } else {
            setCurrentIndex(currentIndex + 1);
        }
        break;
    default:
        event->ignore();
        break;
    }

    return QScrollArea::keyPressEvent(event);
}

void WallpaperList::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    if (width() < kItemWidth) {
        fmCritical() << "error. widget width is less than ItemWidth" << width() << "<" << kItemWidth
                     << "resize" << event->size();
    }

    int itemCount = width() / kItemWidth;
    if (width() % kItemWidth == 0)
        --itemCount;

    if (itemCount < 1) {
        fmCritical() << "screen_item_count: " << itemCount
                     << "set to 1";
        itemCount = 1;
    }

    setGridSize(QSize(width() / itemCount, kItemHeight));
}

void WallpaperList::wheelEvent(QWheelEvent *event)
{
    Q_UNUSED(event);
    // why?
}

void WallpaperList::showEvent(QShowEvent *event)
{
    updateBothEndsItem();
    return QScrollArea::showEvent(event);
}

QWidget *WallpaperList::itemAt(int idx) const
{
    if (idx >= contentLayout->count() || idx < 0) {
        fmCritical() << "error index" << idx << "gridsie" << grid << geometry() << contentLayout->count();
        return nullptr;
    }

    return contentLayout->itemAt(idx)->widget();
}

void WallpaperList::updateBothEndsItem()
{
    int currentValue = horizontalScrollBar()->value();

    if (prevItem)
        prevItem->setOpacity(1);

    if (nextItem)
        nextItem->setOpacity(1);

    if (contentLayout->isEmpty()) {
        prevItem = nullptr;
        nextItem = nullptr;
    } else {
        prevItem = qobject_cast<WallpaperItem *>(itemAt(kItemWidth / 2, kItemHeight / 2));
        nextItem = qobject_cast<WallpaperItem *>(itemAt(width() - kItemWidth / 2, kItemHeight / 2));
    }

    if (currentValue == horizontalScrollBar()->minimum())
        prevItem = nullptr;
    if (currentValue == horizontalScrollBar()->maximum())
        nextItem = nullptr;

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

void WallpaperList::showDeleteButtonForItem(const WallpaperItem *item) const
{
    if (item && item->isDeletable() && item != prevItem && item != nextItem) {
        // we can't get a correct item if content image geometry is not available to use.
        if (item->contentGeometry().isNull()) {
            return;
        }
        emit mouseOverItemChanged(item->itemData(),
                                  item->mapTo(parentWidget(),
                                              item->contentGeometry().topRight() / devicePixelRatioF()));
    } else {
        emit mouseOverItemChanged("", QPoint(0, 0));
    }
}

void WallpaperList::scrollList(int step, int duration)
{
    if (scrollAnimation.state() == QAbstractAnimation::Running)
        return;

    int startValue = horizontalScrollBar()->value();
    int endValue = startValue + step;

    if ((endValue < horizontalScrollBar()->minimum() && startValue == horizontalScrollBar()->minimum())
        || (endValue > horizontalScrollBar()->maximum() && startValue == horizontalScrollBar()->maximum())) {
        return;
    }

    scrollAnimation.setDuration(duration);
    scrollAnimation.setStartValue(startValue);
    scrollAnimation.setEndValue(endValue);
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

void WallpaperList::updateItemThumb()
{
    contentWidget->adjustSize();

    showDeleteButtonForItem(static_cast<WallpaperItem *>(itemAt(mapFromGlobal(QCursor::pos()))));
    QRect r = rect();
    QRect cacheRect(r.x() - r.width(), r.y(), r.width() * 3, r.height());
    for (WallpaperItem *item : items) {
        if (cacheRect.intersects(QRect(item->mapTo(this, QPoint()), item->size()))) {
            item->renderPixmap();
        }
    }

    updateBothEndsItem();
}

void WallpaperList::init()
{
    prevButton = new DIconButton(DDciIcon(QStringLiteral(":/images/arrow-left.dci")), this);
    nextButton = new DIconButton(DDciIcon(QStringLiteral(":/images/arrow-right.dci")), this);

    setObjectName("WallpaperList-QScrollArea");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAttribute(Qt::WA_TranslucentBackground);
    setFrameShape(QFrame::NoFrame);
    viewport()->setAutoFillBackground(false);
    horizontalScrollBar()->setEnabled(false);
    setFocusPolicy(Qt::NoFocus);

    updateTimer = new QTimer(this);
    updateTimer->setInterval(100);
    updateTimer->setSingleShot(true);

    connect(updateTimer, &QTimer::timeout, this, &WallpaperList::updateItemThumb);
    connect(&scrollAnimation, &QAbstractAnimation::finished, updateTimer, static_cast<void (QTimer::*)()>(&QTimer::start));

    contentWidget = new QWidget(this);
    contentWidget->setAttribute(Qt::WA_TranslucentBackground);
    contentLayout = new QHBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    setWidget(contentWidget);
    contentWidget->setAutoFillBackground(false);

    prevButton->hide();
    prevButton.setAnchor(Qt::AnchorVerticalCenter, this, Qt::AnchorVerticalCenter);
    prevButton.setAnchor(Qt::AnchorLeft, this, Qt::AnchorLeft);
    nextButton->hide();
    nextButton.setAnchor(Qt::AnchorVerticalCenter, this, Qt::AnchorVerticalCenter);
    nextButton.setAnchor(Qt::AnchorRight, this, Qt::AnchorRight);

    connect(prevButton.widget(), &DIconButton::clicked, this, &WallpaperList::prevPage);
    connect(nextButton.widget(), &DIconButton::clicked, this, &WallpaperList::nextPage);

    scrollAnimation.setTargetObject(horizontalScrollBar());
    scrollAnimation.setPropertyName("value");
    scrollAnimation.setDuration(QEasingCurve::OutExpo);
}
